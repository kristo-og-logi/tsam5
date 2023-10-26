// Client.h

#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <queue>

enum class ClientType { SERVER, CLIENT };

class Client {
  public:
    int sock; // socket of client connection
    ClientType type;
    std::string name; // Limit length of name of client's user
    std::string ip;
    int port;
    std::queue<std::string> messages;
  

    Client(int socket, ClientType clientType, std::string ip, int port);
    virtual ~Client(); // Virtual destructor if intended to be a base class

    // Convert the 'type' member variable to its corresponding string
    std::string clientTypeToString() const;
    std::string toString() const;

  bool operator<(const Client& other) const {
        return name < other.name;
    }
  bool operator==(const Client& other) const {
        return this->name == other.name;
    }
};

#endif // CLIENT_H
