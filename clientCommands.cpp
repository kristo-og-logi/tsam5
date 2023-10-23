#include <iostream>     // for std::cout + endl
#include <set>          // for std::set
#include <string>       // for std::string
#include <sys/socket.h> // for socket, listen, send

#include "Client.h"
#include "clientCommands.h"

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