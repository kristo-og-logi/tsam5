// Client.cpp

#include "Client.h"

Client::Client(int socket, ClientType clientType, std::string ip, int port)
    : sock(socket), type(clientType), ip(ip), port(port) {}

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
