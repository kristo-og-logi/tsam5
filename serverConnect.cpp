#include <iostream>
#include <string>
#include <sys/socket.h>

#include "Client.h"
#include "ip.h"
#include "serverConnect.h"

Client *connectToServer(std::string &data, int serverPort) {
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
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Failed to create socket: " << std::strerror(errno)
                  << std::endl;
        return nullptr;
    }

    // Define the server address
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
        -1) {
        std::cerr << "Failed to connect: " << std::strerror(errno) << std::endl;
        close(sock);
        return nullptr;
    }

    std::cout << "Connected to server " << ip << ":" << port << std::endl;

    std::string message = "QUERYSERVERS,P3_GROUP_6," + getMyIp() + "," +
                          std::to_string(serverPort) + "\n";

    std::cout << "sent (" << sock << "): " << message << std::endl;

    message.insert(0, 1, char(0x02));
    message += char(0x03);

    send(sock, message.c_str(), message.size(), 0);

    Client *newClient = new Client(sock, ClientType::SERVER, ip, port);
    return newClient;
}
