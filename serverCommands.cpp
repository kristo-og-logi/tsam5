#include <iostream> // for std::cout + endl
#include <queue>
#include <set> // set
#include <sstream>
#include <string>       // for std::string
#include <sys/socket.h> // for socket, listen, send
#include <vector>

#include "Client.h"
#include "ServerSettings.h"
#include "ip.h"
#include "sendMessage.h"
#include "serverCommands.h"
#include "serverConnect.h"
#include "utils.h"

std::vector<unsigned char>
constructMessage(std::vector<std::string> command_msg) {
    unsigned char prefix = 0x02;
    unsigned char postfix = 0x03;
    std::vector<unsigned char> buffer;

    buffer.push_back(prefix);

    for (size_t i = 0; i < command_msg.size(); i++) {
        if (i > 0) {
            buffer.push_back(',');
        }
        buffer.insert(buffer.end(), command_msg[i].begin(),
                      command_msg[i].end());
    }

    buffer.push_back(postfix);

    return buffer;
}

void handleERROR(int socket, const std::string message) {
    std::cerr << socket << "| ERROR received: " << message << std::endl;
    return;
}

void handleSERVERS(int socket, const std::string data,
                   std::set<Client *> &servers, std::set<Client *> &newServers,
                   ServerSettings &groupSixServer, int serverPort) {
    int commaIndex = 0;

    std::vector<std::string> oneHopServers = splitString(data, ';');

    if (oneHopServers.empty()) {
        std::cerr << socket << "| ERROR: SERVERS response empty" << std::endl;
        return;
    }

    std::string owner = oneHopServers.front();
    std::vector<std::string> ownerData = splitString(owner, ',');

    if (ownerData.size() != 3) {
        std::cerr << socket
                  << "| ERROR: Invalid number of owner attributes in owner "
                     "SERVERS response. Should be <name>,<ip>,<port> but is "
                  << owner << std::endl;
        return;
    }

    std::string ownerName = ownerData[0];
    std::string ownerIp = ownerData[1];
    std::string ownerPort = ownerData[2];

    int port = -1;
    if (!isConvertibleToInt(ownerPort, port)) {
        std::cerr << socket << "| INVALID servers response - Port " << ownerPort
                  << " invalid" << std::endl;
        return;
    }

    for (Client *server : servers)
        if (server->sock == socket)
            server->port = port;

    for (auto s = oneHopServers.begin() + 1; s != oneHopServers.end(); ++s) {

        if (servers.size() + newServers.size() >= groupSixServer.maxConnections)
            break;

        std::vector<std::string> serverData = splitString(*s, ',');
        if (serverData.size() != 3) // if there aren't three items, stop
            continue;

        if (serverData[0] ==
            groupSixServer.serverName) // if the server is us, stop
            continue;

        int serverDataPort;
        if (!isConvertibleToInt(
                serverData[2],
                serverDataPort)) // if the port is not an int, stop
            continue;

        if (serverDataPort < 0) // if the port is < 0, stop
            continue;

        // are we already connected to this server
        bool isValid = true;
        for (Client *connectedServer : servers)
            if (connectedServer->name == serverData[0]) {
                isValid = false;
                break;
            }

        if (!isValid)
            continue;

        // We can't trust that within a single SERVERS response, two of the same
        // servers can't appear
        for (Client *unconnectedServer : newServers)
            if (unconnectedServer->ip == serverData[1] &&
                unconnectedServer->port == serverDataPort) {
                isValid = false;
                break;
            }

        if (!isValid)
            continue;

        Client *newClient = establishConnection(serverData[1], serverDataPort,
                                                serverPort, groupSixServer);
        if (newClient != nullptr)
            newServers.insert(newClient);
    }
    return;
}

void sendKEEPALIVE(std::set<Client *> servers) {
    for (Client *server : servers) {
        std::vector<std::string> message = {
            "KEEPALIVE," + std::to_string(server->messages.size())};
        std::vector<unsigned char> keepalive = constructMessage(message);

        // send(server->sock, message.data(), message.size(), 0);
        std::string result(keepalive.begin(), keepalive.end());
        sendMessage(server->sock, result);
    }
}

void handleQUERYSERVERS(int socket, const std::string data,
                        const std::set<Client *> &servers, int serverPort,
                        ServerSettings &groupSixServer) {
    std::string response = "SERVERS," + groupSixServer.serverName + "," +
                           getMyIp() + "," + std::to_string(serverPort) + ";";

    int commaIndex = data.find(",");

    std::string name = data;

    if (commaIndex != std::string::npos)
        name = data.substr(0, commaIndex);

    for (Client *server : servers) {
        if (server->sock == socket)
            server->name = name;
        response += server->toString();
    }

    // send(socket, serverResponse.data(), serverResponse.size(), 0);
    sendMessage(socket, response);
}

void handleFETCH_MSGS(int socket, const std::string data,
                      const std::set<Client *> &servers) {
    // Take the Group ID, see if it exists in known servers
    // Send messages until queue is empty

    int knownServer = 0;
    Client *friendlyServer;

    for (Client *server : servers) {
        if (server->name == data) {
            knownServer = 1;
            friendlyServer = server;
        }
    }

    if (knownServer == 1) {
        while (!friendlyServer->messages.empty()) {
            std::string msg = friendlyServer->messages.front();
            std::cout << msg << std::endl;

            friendlyServer->messages.pop();

            std::vector<std::string> tmp_msg = {msg};
            std::vector<unsigned char> buffer = constructMessage(tmp_msg);

            std::string result(buffer.begin(), buffer.end());
            sendMessage(socket, result);
            // send(socket, buffer.data(), buffer.size(), 0);
        }
    }

    else if (knownServer == 0) {
        std::vector<std::string> msg = {"No messages for " + data};
        std::vector<unsigned char> buffer = constructMessage(msg);

        // send(socket, buffer.data(), buffer.size(), 0);
        std::string result(buffer.begin(), buffer.end());
        sendMessage(socket, result);
    }

    return;
}

void handleSEND_MSG(int socket, const std::string data,
                    const std::set<Client *> &servers,
                    std::set<Client *> &unknownServers,
                    ServerSettings &myServer) {

    int messageSent = 0;

    std::string toGroup, fromGroup;
    std::string command = "SEND_MSG";

    std::stringstream ss(data);
    std::getline(ss, toGroup, ',');
    std::getline(ss, fromGroup, ',');

    std::string content((std::istreambuf_iterator<char>(ss)),
                        std::istreambuf_iterator<char>());

    if (toGroup == myServer.serverName) {
        myServer.addMessage(fromGroup, content);
        std::cout << fromGroup << ":" << content << std::endl;
        return;
    }

    std::vector<int> instructorSockets = {};
    std::vector<std::string> command_msg = {command, toGroup, fromGroup,
                                            content};
    std::vector<unsigned char> buffer = constructMessage(command_msg);

    for (Client *server : servers) {

        if (server->name == toGroup) {
            std::vector<unsigned char> success =
                constructMessage({"Successfully sent message"});

            // send(server->sock, buffer.data(), buffer.size(), 0);
            std::string result(buffer.begin(), buffer.end());
            sendMessage(server->sock, result);

            // send(socket, success.data(), success.size(), 0);
            std::string successResult(success.begin(), success.end());
            sendMessage(socket, successResult);
            messageSent = 1;
        }
        if (server->name.find("Instr_") != std::string::npos) {
            instructorSockets.push_back(server->sock);
        }
    }

    if (messageSent == 0) {
        if (instructorSockets.size() != 0) {
            std::vector<unsigned char> response = constructMessage(
                {"Sent messages to Instructor, GROUP was unknown"});
            std::string updateResult(response.begin(), response.end());

            std::string result(buffer.begin(), buffer.end());

            for (int instrSock : instructorSockets) {
                sendMessage(instrSock, result);
            }

            sendMessage(socket, updateResult);
        } else {
            std::vector<unsigned char> response =
                constructMessage({"Could not send message, GROUP was unknown"});

            std::string result(response.begin(), response.end());
            sendMessage(socket, result);
        }

        for (Client *unkownServer : unknownServers) {
            if (unkownServer->name == toGroup) {
                unkownServer->messages.push(command + "," + data);
                return;
            }
        }

        Client *unknown = new Client(-1, ClientType::SERVER, "NA", -1);
        unknown->name = toGroup;
        unknown->messages.push(command + "," + data);
        unknownServers.insert(unknown);
    }

    return;
}

void handleSTATUSREQ(int socket, const std::string data,
                     const std::set<Client *> &servers,
                     ServerSettings &myServer) {
    return handleSTATUSRESP(socket, data, servers, myServer);
}

void handleSTATUSRESP(int socket, const std::string data,
                      const std::set<Client *> &servers,
                      ServerSettings &myServer) {
    int foundServer;
    std::vector<std::string> serversAndMessages = {};
    std::vector<std::string> resBuilder{"STATUSRESP", myServer.serverName};

    if (data != myServer.serverName) {
        return;
    }

    for (Client *server : servers) {

        serversAndMessages.push_back(server->name);
        serversAndMessages.push_back(std::to_string(server->messages.size()));

        if (server->sock == socket) {
            resBuilder.push_back(server->name);
            foundServer = 1;
        }
    }

    resBuilder.insert(resBuilder.end(), serversAndMessages.begin(),
                      serversAndMessages.end());

    std::vector<unsigned char> res = constructMessage(resBuilder);

    if (foundServer == 1) {
        // send(socket, res.data(), res.size(), 0);
        std::string result(res.begin(), res.end());
        sendMessage(socket, result);
    }

    return;
}

void sendFETCH_MSGS(int socket, ServerSettings myServer) {
    std::vector<std::string> messageBuilder = {"FETCH_MSGS," +
                                               myServer.serverName};
    std::vector<unsigned char> message = constructMessage(messageBuilder);

    std::string result(message.begin(), message.end());
    sendMessage(socket, result);

    return;
}

void handleKEEPALIVE(int socket, const std::string data,
                     const std::set<Client *> &servers,
                     ServerSettings &myServer) {
    if (data == "0") {
        return;
    }

    // for (Client *server : servers) {
    //     if (server->sock == socket) {
    return sendFETCH_MSGS(socket, myServer);
}

void handleUNSUPPORTED(int socket, const std::string command,
                       const std::string data) {
    std::cout << socket << "| unsupported server command: " << command
              << " received" << std::endl;

    return;
}
