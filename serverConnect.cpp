#include <iostream>
#include <set>
#include <string>
#include <sys/socket.h>

#include "Client.h"
#include "ServerSettings.h"
#include "createSocket.h"
#include "ip.h"
#include "sendMessage.h"
#include "serverConnect.h"
#include "utils.h"

void sendQUERYSERVERS(int serverPort, int sock, ServerSettings &myServer) {
    std::string message = "QUERYSERVERS," + myServer.serverName + "," +
                          getMyIp() + "," + std::to_string(serverPort);

    message.insert(0, 1, char(0x02));
    message += char(0x03);

    // send(sock, message.c_str(), message.size(), 0);
    sendMessage(sock, message);
}

// takes strings with format "<ip>,<port>" and creates connection through that
// and sends QUERYSERVERS request
Client *connectToServer(std::string &data, int serverPort,
                        ServerSettings &myServer, std::set<Client *> &servers) {
    size_t firstCommaIndex = data.find(",");

    if (firstCommaIndex == std::string::npos) {
        std::cout << "Invalid Connection data" << std::endl;
        return nullptr;
    }
    std::string ip = data.substr(0, firstCommaIndex);

    std::string userPort = data.substr(firstCommaIndex + 1, data.size());
    int port;

    if (!isConvertibleToInt(userPort, port))
        std::cerr << "ERROR: invalid port, unable to connect to " << ip << ":"
                  << userPort;

    // what's the mininum valid port?
    // I thought 1024, but I was allowed to boot up the server with port 1000
    if (port < 0) {
        std::cerr << "ERROR: invalid port, should be >= 0 but is " << port
                  << std::endl;
        return nullptr;
    }

    for (auto const *s : servers)
        if (s->ip == ip && s->port == port) {
            std::cerr << "ERROR: Attempted duplicate connection with " << ip
                      << ":" << port << std::endl;
            return nullptr;
        }

    // Create a socket
    struct sockaddr_in addr;
    int sock = createConnection(ip, port, addr);
    if (sock < 0)
        return nullptr;

    sendQUERYSERVERS(serverPort, sock, myServer);

    Client *newClient = new Client(sock, ClientType::SERVER, ip, port);
    newClient->name = "unknown" + std::to_string(sock);

    return newClient;
}
