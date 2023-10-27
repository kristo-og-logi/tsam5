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
#include "serverCommands.h"

const std::string GROUP_NAME = "P3_GROUP_6";

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

    buffer.push_back('\n');
    buffer.push_back(postfix);

    return buffer;
}

void handleSERVERS(int socket, const std::string data) {
    std::cout << "Received (" << socket << "): SERVERS," << data << std::endl;
    return;
}

void sendKEEPALIVE(std::set<Client *> servers) {
    for (Client *server : servers) {
        std::vector<std::string> message = {
            "KEEPALIVE," + std::to_string(server->messages.size())};
        std::vector<unsigned char> keepalive = constructMessage(message);

        send(server->sock, message.data(), message.size(), 0);
    }
}

void handleQUERYSERVERS(int socket, const std::string data,
                        const std::set<Client *> &servers, int serverPort) {
    std::cout << "Received (" << socket << "): QUERYSERVERS," << data
              << std::endl;

    std::string response = "SERVERS," + GROUP_NAME + "," + getMyIp() + "," +
                           std::to_string(serverPort) + ";";

    for (Client *server : servers) {
        if (server->sock == socket)
            server->name = data;
        response += server->toString();
    }

    response += "\n";
    std::vector<std::string> serverBuilder = {response};
    std::vector<unsigned char> serverResponse = constructMessage(serverBuilder);

    std::cout << "responds (" << socket << "): " << response << std::endl;

    send(socket, serverResponse.data(), serverResponse.size(), 0);
    return;
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

            send(socket, buffer.data(), buffer.size(), 0);
        }
    }

    if (knownServer == 0) {
        std::vector<std::string> msg = {"No messages for" + data};
        std::vector<unsigned char> buffer = constructMessage(msg);
        send(socket, buffer.data(), buffer.size(), 0);
    }

    return;
}

void handleSEND_MSG(int socket, const std::string data,
                    const std::set<Client *> &servers,
                    std::set<Client *> &unknownServers,
                    ServerSettings myServer) {

    int messageSent = 0;

    std::string toGroup, fromGroup, content;
    std::string command = "SEND_MSG";

    std::stringstream ss(data);
    std::getline(ss, toGroup, ',');
    std::getline(ss, fromGroup, ',');
    std::getline(ss, content);

    if (toGroup == myServer.serverName) {
        myServer.addMessage(fromGroup + ":" + content);
        std::cout << fromGroup << ":" << content << std::endl;
        return;
    }

    std::vector<std::string> command_msg = {command, toGroup, fromGroup,
                                            content};
    std::vector<unsigned char> buffer = constructMessage(command_msg);

    for (Client *server : servers) {

        if (server->name == toGroup) {
            std::vector<unsigned char> success =
                constructMessage({"Successfully sent message"});

            send(server->sock, buffer.data(), buffer.size(), 0);
            send(socket, success.data(), success.size(), 0);
            messageSent = 1;
        }
    }

    if (messageSent == 0) {
        std::vector<unsigned char> response =
            constructMessage({"Could not send message"});
        send(socket, response.data(), response.size(), 0);

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
                     ServerSettings myServer) {
    return handleSTATUSRESP(socket, data, servers, myServer);
}

void handleSTATUSRESP(int socket, const std::string data,
                      const std::set<Client *> &servers,
                      ServerSettings myServer) {
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
        send(socket, res.data(), res.size(), 0);
    }

    return;
}

void parseMessages(const char *buffer, ssize_t length,
                   ServerSettings myServer) {
    const char STX = 0x02;
    const char ETX = 0x03;

    static std::string currentMessage;
    static bool inMessage = false;

    for (ssize_t i = 0; i < length; ++i) {
        if (buffer[i] == STX) {
            inMessage = true;
            currentMessage.clear();
        } else if (buffer[i] == ETX && inMessage) {
            inMessage = false;
            myServer.addMessage(currentMessage);
        } else if (inMessage) {
            currentMessage.push_back(buffer[i]);
        }
    }
}

void sendFETCH_MSGS(int socket, ServerSettings myServer) {
    char buffer[5000];
    std::string message = "FETCH_MSGS," + myServer.serverName;

    ssize_t bytesSent = send(socket, message.c_str(), message.size(), 0);

    if (bytesSent < 0) {
        return;
    }

    ssize_t bytesReceived = recv(socket, buffer, sizeof(buffer), 0);

    if (bytesReceived > 0) {
        parseMessages(buffer, bytesReceived, myServer);
    }
    return;
}

void handleKEEPALIVE(int socket, const std::string data,
                     const std::set<Client *> &servers,
                     ServerSettings myServer) {
    std::cout << "Received (" << socket << "): KEEPALIVE," << data << std::endl;

    if (data == "0") {
        return;
    }

    for (Client *server : servers) {
        if (server->sock == socket) {
            return sendFETCH_MSGS(socket, myServer);
        }
    }

    return;
}

void handleUNSUPPORTED(int socket, const std::string command,
                       const std::string data) {
    std::cout << "unsupported server command: " << command << " received"
              << std::endl;

    std::string response = "UNSUPPORTED, P3_GROUP_6";
    send(socket, response.c_str(), response.size(), 0);

    return;
}
