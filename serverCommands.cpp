#include <iostream>     // for std::cout + endl
#include <string>       // for std::string
#include <sys/socket.h> // for socket, listen, send

#include "serverCommands.h"

void handleQUERYSERVERS(int socket, std::string data) {
    std::cout << data << std::endl;

    std::string response = "SERVERS, P3_GROUP_6\n";
    send(socket, response.c_str(), response.size(), 0);

    return;
}

// int main() {
//     handleQUERYSERVERS("Test data");
//     return 0;
// }