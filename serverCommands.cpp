#include <iostream>     // for std::cout + endl
#include <set>          // set
#include <string>       // for std::string
#include <sys/socket.h> // for socket, listen, send

#include "Client.h"
#include "serverCommands.h"

void handleKEEPALIVE(int socket, const std::string data) {
    std::cout << "keepalive: " << data << std::endl;

    std::string response = "KEEPALIVE, P3_GROUP_6\n";
    send(socket, response.c_str(), response.size(), 0);

    return;
}

void handleQUERYSERVERS(int socket, const std::string data,
                        const std::set<Client *> &servers) {
    std::cout << data << std::endl;

    std::string response = "SERVERS,";
    std::string serverString;

    for (Client *server : servers) {
        if (server->sock == socket) {
            server->name = data;
            response += server->toString();
        } else {
            serverString += server->toString();
        }
    }

    response += serverString + "\n";
    send(socket, response.c_str(), response.size(), 0);

    return;
}

void handleFETCH_MSGS(int socket, const std::string data) {
    std::cout << "FETCH_MSGS: " << data << std::endl;

    std::string response = "FETCH_MSGS, P3_GROUP_6\n";
    send(socket, response.c_str(), response.size(), 0);

    return;
}

void handleSEND_MSG(int socket, const std::string data) {
    std::cout << "SEND_MSG: " << data << std::endl;

    std::string response = "SEND_MSG, P3_GROUP_6\n";
    send(socket, response.c_str(), response.size(), 0);

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

// int main() {
//     handleQUERYSERVERS("Test data");
//     return 0;
// }