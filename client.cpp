// Client.cpp

#include "Client.h"
#include <queue>

Client::Client(int socket, ClientType clientType, std::string ip, int port)
    : sock(socket), type(clientType), ip(ip), port(port), messages() {}

Client::~Client() {}

std::string Client::clientTypeToString() const {
    switch (type) {
    case ClientType::SERVER:
        return "server";
    case ClientType::CLIENT:
        return "client";
    default:
        return "unknown";
    }
}

std::string Client::toString() const {
    return name + "," + ip + "," + std::to_string(port) + ";";
}
