#include <iostream> // for std::cout + endl
#include <ostream>
#include <set> // for std::set
#include <sstream>
#include <string>       // for std::string
#include <sys/socket.h> // for socket, listen, send

#include "Client.h"
#include "ServerSettings.h"
#include "clientCommands.h"
#include "serverCommands.h"
#include "serverConnect.h"

void handleLISTSERVERS(int socket, std::set<Client *> &servers) {
    std::cout << "listservers received" << std::endl;

    std::string response;

    for (const Client *s : servers) {
        response +=
            s->name + "," + s->ip + "," + std::to_string(s->port) + ";\n";
    }

    send(socket, response.c_str(), response.size(), 0);
    return;
}

Client *handleCONNECT(int socket, std::string data, int serverPort,
                      ServerSettings &myServer) {
    Client *newClient = connectToServer(data, serverPort, myServer);
    return newClient;
}

void handleGETMSG(int socket, std::string group, ServerSettings &myServer) {

    if (group == myServer.serverName) {
        std::string message = "Cannot send message to self";
        send(socket, message.c_str(), message.size(), 0);
        return;
    }

    std::string message = myServer.getMessage(group);

    if (message == "") {
        std::string failMsg = "No messages from " + group;
        send(socket, failMsg.c_str(), failMsg.size(), 0);
        return;
    }

    send(socket, message.c_str(), message.size(), 0);
    return;
}

void handleSENDMSG(int socket, std::string data, std::set<Client *> &servers,
                   std::set<Client *> &unknownServers,
                   ServerSettings &myServer) {

    std::string group, message;

    std::stringstream ss(data);
    std::getline(ss, group, ',');
    std::getline(ss, message);

    std::string completeCommand =
        group + "," + myServer.serverName + "," + message;

    return handleSEND_MSG(socket, completeCommand, servers, unknownServers,
                          myServer);
}

void handleUNSUPPORTEDCLIENT(int socket, std::string command) {
    std::cout << "unsupported client command: " << command << " received"
              << std::endl;
}
