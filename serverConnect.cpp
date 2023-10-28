#include <iostream>
#include <string>
#include <sys/socket.h>

#include "Client.h"
#include "createSocket.h"
#include "ip.h"
#include "serverConnect.h"
#include "sendMessage.h"
#include "ServerSettings.h"

void sendQUERYSERVERS(int serverPort, int sock, ServerSettings &myServer) {
    std::string message = "QUERYSERVERS," + myServer.serverName + "," + getMyIp() + "," +
                          std::to_string(serverPort);

    message.insert(0, 1, char(0x02));
    message += char(0x03);

    // send(sock, message.c_str(), message.size(), 0);
	sendMessage(sock, message);
}

Client *connectToServer(std::string &data, int serverPort, ServerSettings &myServer) {
    size_t firstCommaIndex = data.find(",");

    if (firstCommaIndex == std::string::npos) {
        std::cout << "Invalid Connection data" << std::endl;
        return nullptr;
    }
    std::string ip = data.substr(0, firstCommaIndex);
    int port = std::stoi(
        data.substr(firstCommaIndex + 1, data.size())); // TODO this could crash

    // TODO check whether this server is already connected

    // Create a socket
    struct sockaddr_in addr;
    int sock = createConnection(ip, port, addr);
    if (sock < 0)
        return nullptr;

    sendQUERYSERVERS(serverPort, sock, myServer);

    Client *newClient = new Client(sock, ClientType::SERVER, ip, port);
    return newClient;
}
