#include <arpa/inet.h> // for inet_ntoa
#include <cstring>
#include <iostream>     // for std::cout + endl + cerr
#include <list>         // for std::list
#include <netinet/in.h> // for sockaddr_in
#include <set>          // for storing sockets
#include <sstream>      // for stream()
#include <sys/socket.h> // for socket, listen, send
#include <unistd.h>     // for close

// fix SOCK_NONBLOCK for OSX
#ifndef SOCK_NONBLOCK
#include <fcntl.h>
#define SOCK_NONBLOCK O_NONBLOCK
#endif

// importing helper files

#include "Client.h"
#include "clientCommands.h"
#include "serverCommands.h"

std::set<Client *> servers; // Lookup table for servers
std::set<Client *> clients; // Lookup table for clients

int createSocket(int portno, struct sockaddr_in addr) {
    socklen_t addr_len = sizeof(addr);

#ifdef __APPLE__
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }
#else
    if ((sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
        perror("Failed to open socket");
        return (-1);
    }
#endif

    int set = 1; // for setsockopt
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set)) < 0) {
        perror("Failed to set SO_REUSEADDR:");
    }
    set = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &set, sizeof(set)) < 0) {
        perror("Failed to set SO_REUSEADDR:");
    }

#ifdef __APPLE__
    set = 1;
    if (setsockopt(sock, SOL_SOCKET, SOCK_NONBLOCK, &set, sizeof(set)) < 0) {
        perror("Failed to set SOCK_NOBBLOCK");
    }
#endif

    // Setup server address structure
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(portno);

    // Bind socket
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(sock);
        perror("Failed to bind socket.");
    }

    // Listen
    if (listen(sock, 5) == -1) {
        close(sock);
        perror("Listen failed on socket.");
    }

    return sock;
}

void closeClient(Client *const &client, fd_set *openSockets, int *maxfds,
                 int *basemaxfds) {
    std::cout << client->clientTypeToString() << " " << client->sock
              << " disconnected" << std::endl;

    close(client->sock);

    if (*maxfds == client->sock) {
        *maxfds = *basemaxfds; // reinitialize the max fd
        // check the clients for maxfd
        for (Client *c : clients)
            if (client->sock != c->sock)
                *maxfds = std::max(*maxfds, c->sock);

        // check the servers for maxfd
        for (Client *s : servers)
            if (client->sock != s->sock)
                *maxfds = std::max(*maxfds, s->sock);
    }

    // remove the socket from the list of open sockets.
    FD_CLR(client->sock, openSockets);
}

bool bufferIsValid(char *buffer) {
    const char FIRST_BYTE = 0x02;
    const char LAST_BYTE = 0x03;
    int msgLength = strlen(buffer);

    // std::cout << "1st check: " << buffer[0] << " = " << FIRST_BYTE <<
    // std::endl; std::cout << "2nd check: " << buffer[msgLength - 1] << " = "
    // << LAST_BYTE << std::endl;

    return !(msgLength < 2 || buffer[0] != FIRST_BYTE ||
             buffer[msgLength - 1] != LAST_BYTE);
}

void clientCommand(int clientSocket, fd_set *openSockets, int *maxfds,
                   char *buffer) {
    std::vector<std::string> tokens;
    std::string token;
    const char *invalidMessage = "invalid message\n";
    int msgLength = strlen(buffer);

    if (!bufferIsValid(buffer)) {
        std::cout << invalidMessage;
        send(clientSocket, invalidMessage, strlen(invalidMessage), 0);
        return;
    }

    std::string content(buffer + 1, msgLength - 2);

    if (content == "LISTSERVERS")
        return handleLISTSERVERS(clientSocket);

    // under here, we should only have commands which must include comma's.
    size_t firstCommaIndex = content.find(',');

    if (firstCommaIndex == std::string::npos) {
        std::cout << invalidMessage;
        send(clientSocket, invalidMessage, strlen(invalidMessage), 0);
        return;
    }

    std::string command = content.substr(0, firstCommaIndex);
    std::string data = content.substr(firstCommaIndex + 1, content.size() - 1);

    if (command == "GETMSG") {
        std::cout << "getmsg received" << std::endl;
    } else if (command == "SENDMSG") {
        std::cout << "sendmsg received" << std::endl;
    } else {
        std::cout << "unsupported client command: " << command << " received"
                  << std::endl;
    }
}

void serverCommand(int serverSocket, fd_set *openSockets, int *maxfds,
                   char *buffer) {
    std::vector<std::string> tokens;
    std::string token;
    const char *invalidMessage = "invalid message\n";
    int msgLength = strlen(buffer);

    if (!bufferIsValid(buffer)) {
        std::cout << invalidMessage;
        send(serverSocket, invalidMessage, strlen(invalidMessage), 0);
        return;
    }

    std::string content(buffer + 1, msgLength - 2);
    size_t firstCommaIndex = content.find(',');

    if (firstCommaIndex == std::string::npos) {
        std::cout << invalidMessage;
        send(serverSocket, invalidMessage, strlen(invalidMessage), 0);
        return;
    }

    std::string command = content.substr(0, firstCommaIndex);
    std::string data = content.substr(firstCommaIndex + 1, content.size() - 1);

    if (command == "KEEPALIVE")
        return handleKEEPALIVE(serverSocket, data);

    else if (command == "QUERYSERVERS")
        return handleQUERYSERVERS(serverSocket, data);

    else if (command == "FETCH_MSGS")
        return handleFETCH_MSGS(serverSocket, data);

    else if (command == "SEND_MSG")
        return handleSEND_MSG(serverSocket, data);

    else if (command == "STATUSREQ")
        return handleSTATUSREQ(serverSocket, data);

    else if (command == "STATUSRESP")
        return handleSTATUSRESP(serverSocket, data);

    else
        return handleUNSUPPORTED(serverSocket, command, data);
}

void acceptConnection(int socket, sockaddr_in socketAddress,
                      fd_set *openSockets, int *maxfds, ClientType clientType) {
    socklen_t clientLen = sizeof(socketAddress);
    int newSocketConnection =
        accept(socket, (struct sockaddr *)&socketAddress, &clientLen);
    if (newSocketConnection == -1) {
        std::cerr << "Failed to accept client." << std::endl;
        return;
    }

    // Retrieve the IP address and port
    std::string clientIP = inet_ntoa(socketAddress.sin_addr);
    uint16_t clientPort = ntohs(socketAddress.sin_port);

    // Add new client to the list of open sockets
    FD_SET(newSocketConnection, openSockets);
    *maxfds = std::max(*maxfds, newSocketConnection);

    Client *newClient =
        new Client(newSocketConnection, clientType, clientIP, clientPort);

    if (clientType == ClientType::CLIENT)
        clients.insert(newClient);
    else
        servers.insert(newClient);

    std::cout << newClient->clientTypeToString() << " " << newSocketConnection
              << " connected from " << clientIP << ":" << clientPort
              << std::endl;

    // Send a message to the client
    const char *message = "Hello!\n";
    send(newSocketConnection, message, strlen(message), 0);
};

void handleClientMessage(Client *const &client, char *buffer, int bufferSize,
                         fd_set *openSockets,
                         std::list<Client *> &disconnectedClients, int *maxfds,
                         int *basemaxfds) {
    if (recv(client->sock, buffer, bufferSize, MSG_DONTWAIT) == 0) {
        disconnectedClients.push_back(client);
        closeClient(client, openSockets, maxfds, basemaxfds);
        return;
    }

    if (strncmp(buffer, "bye", 3) == 0) {
        disconnectedClients.push_back(client);
        closeClient(client, openSockets, maxfds, basemaxfds);
    }

    else
        clientCommand(client->sock, openSockets, maxfds, buffer);
};

void handleServerMessage(Client *const &server, char *buffer, int bufferSize,
                         fd_set *openSockets,
                         std::list<Client *> &disconnectedServers, int *maxfds,
                         int *basemaxfds) {
    // receive the message from the server
    if (recv(server->sock, buffer, bufferSize, MSG_DONTWAIT) == 0) {
        disconnectedServers.push_back(server);
        closeClient(server, openSockets, maxfds, basemaxfds);
        return;
    }

    if (strncmp(buffer, "bye", 3) == 0) {
        disconnectedServers.push_back(server);
        closeClient(server, openSockets, maxfds, basemaxfds);
    }

    else
        serverCommand(server->sock, openSockets, maxfds, buffer);
};

int main(int argc, char *argv[]) {
    fd_set openSockets, readSockets,
        exceptSockets; // open, listed, and exception sockets.
    int basemaxfds, maxfds, socketsReady, serverSocket, clientSocket;
    char buffer[1025]; // buffer for reading from clients

    if (argc != 3) {
        printf("Usage: ./server <server port> <client port>\n");
        exit(0);
    }

    int serverPort = atoi(argv[1]);
    int clientPort = atoi(argv[2]);
    struct sockaddr_in server_addr, client_addr;

    // Create socket
    serverSocket = createSocket(serverPort, server_addr);
    clientSocket = createSocket(clientPort, client_addr);

    std::cout << "Server listening on port " << serverPort
              << " for servers and port " << clientPort << " for clients..."
              << std::endl;

    FD_ZERO(&openSockets);
    FD_SET(serverSocket, &openSockets);
    FD_SET(clientSocket, &openSockets);
    basemaxfds = maxfds = std::max(serverSocket, clientSocket);

    bool finished = false;
    while (!finished) {
        // Get modifiable copy of readSockets
        readSockets = exceptSockets = openSockets;
        memset(buffer, 0, sizeof(buffer)); // Initialize the buffer

        // Look at sockets and see which ones have something to be read()
        if ((socketsReady = select(maxfds + 1, &readSockets, NULL,
                                   &exceptSockets, NULL)) < 0) {
            perror("select failed - closing down\n");
            finished = true;
            break;
        }

        if (FD_ISSET(clientSocket,
                     &readSockets)) // we have a new client connection
            acceptConnection(clientSocket, client_addr, &openSockets, &maxfds,
                             ClientType::CLIENT);

        if (FD_ISSET(serverSocket,
                     &readSockets)) // we have a new server connection

            acceptConnection(serverSocket, server_addr, &openSockets, &maxfds,
                             ClientType::SERVER);

        // Now check for commands from clients
        std::list<Client *> disconnectedClients;
        std::list<Client *> disconnectedServers;

        for (auto const &client : clients)
            if (FD_ISSET(client->sock, &readSockets))
                handleClientMessage(client, buffer, sizeof(buffer),
                                    &openSockets, disconnectedClients, &maxfds,
                                    &basemaxfds);

        // Remove client from the clients list
        for (auto const &c : disconnectedClients)
            clients.erase(c);

        for (auto const &server : servers)
            if (FD_ISSET(server->sock, &readSockets))
                handleServerMessage(server, buffer, sizeof(buffer),
                                    &openSockets, disconnectedServers, &maxfds,
                                    &basemaxfds);

        // Remove client from the clients list
        for (auto const &s : disconnectedServers)
            servers.erase(s);
    }

    // Close sockets
    close(serverSocket);
    close(clientSocket);

    return 0;
}
