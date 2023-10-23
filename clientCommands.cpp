#include <iostream>     // for std::cout + endl
#include <string>       // for std::string
#include <sys/socket.h> // for socket, listen, send

#include "clientCommands.h"

void handleLISTSERVERS(int socket) {
    std::cout << "listservers received" << std::endl;

    std::string response = "listservers received\n";
    send(socket, response.c_str(), response.size(), 0);

    return;
}