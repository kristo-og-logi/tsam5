// Client.h

#ifndef CLIENT_H
#define CLIENT_H

#include <string>

enum class ClientType { SERVER, CLIENT };

class Client {
  public:
    int sock; // socket of client connection
    ClientType type;
    std::string name; // Limit length of name of client's user
    std::string ip;
    int port;

    Client(int socket, ClientType clientType, std::string ip, int port);
    virtual ~Client(); // Virtual destructor if intended to be a base class

    // Convert the 'type' member variable to its corresponding string
    std::string clientTypeToString() const;
};

#endif // CLIENT_H
