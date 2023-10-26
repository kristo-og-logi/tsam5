#include <iostream>     // for std::cout + endl
#include <set>          // for std::set
#include <string>       // for std::string
#include <sys/socket.h> // for socket, listen, send

#include "Client.h"
#include "clientCommands.h"
#include "serverConnect.h"

void handleLISTSERVERS(int socket, std::set<Client *> &servers) {
    std::cout << "listservers received" << std::endl;

    std::string response;

    for (const Client *s : servers) {
        response += s->name + "," + s->ip + "," + std::to_string(s->port) + ";";
    }

    response += "\n";

    send(socket, response.c_str(), response.size(), 0);

    return;
}

Client *handleCONNECT(int socket, std::string data, int serverPort) {
    Client *newClient = connectToServer(data, serverPort);
    return newClient;
}

void handleGETMSG(int socket) { std::cout << "getmsg received" << std::endl; }

void handleSENDMSG(int socket) { std::cout << "sendmsg received" << std::endl; }

void handleUNSUPPORTEDCLIENT(int socket, std::string command) {
    std::cout << "unsupported client command: " << command << " received"
              << std::endl;
}
