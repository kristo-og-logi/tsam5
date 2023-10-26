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

void handleKEEPALIVE(int socket, const std::string data) {
    std::cout << "Received (" << socket << "): KEEPALIVE," << data << std::endl;

    std::string response = "KEEPALIVE, P3_GROUP_6\n";
    std::cout << "Responds (" << socket << "): " << response << std::endl;
    send(socket, response.c_str(), response.size(), 0);

    return;
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

    std::cout << "responds (" << socket << "): " << response << std::endl;

    send(socket, response.c_str(), response.size(), 0);
    return;
}

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

    unsigned char prefix = 0x02;
    unsigned char postfix = 0x03;
    int messageSent = 0;

    std::string toGroup, fromGroup, content;
    std::string command = "SEND_MSG";

    std::stringstream ss(data);
    std::getline(ss, toGroup, ',');
    std::getline(ss, fromGroup, ',');
    std::getline(ss, content);

    if (toGroup == myServer.serverName) {
        myServer.addMessage(fromGroup + ":" + content);
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

        Client *unknown = new Client(-1, ClientType::SERVER, "NA", -1);
        unknown->name = toGroup;
        unknown->messages.push(command + "," + data);
        unknownServers.insert(unknown);
    }

    return;
}

void handleSTATUSREQ(int socket, const std::string data) {
    std::cout << "STATUSREQ: " << data << std::endl;

    std::string response = "STATUSREQ, P3_GROUP_6\n";
    send(socket, response.c_str(), response.size(), 0);

    return;
}

void handleSTATUSRESP(int socket, const std::string data) {
    std::cout << "STATUSRESP: " << data << std::endl;

    std::string response = "STATUSRESP, P3_GROUP_6\n";
    send(socket, response.c_str(), response.size(), 0);

    return;
}

void handleUNSUPPORTED(int socket, const std::string command,
                       const std::string data) {
    std::cout << "unsupported server command: " << command << " received"
              << std::endl;

    std::string response = "UNSUPPORTED, P3_GROUP_6\n";
    send(socket, response.c_str(), response.size(), 0);

    return;
}
