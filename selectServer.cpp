#include <algorithm>   // for find_if on linux
#include <arpa/inet.h> // for inet_ntoa
#include <chrono>
#include <cstring>
#include <iostream>     // for std::cout + endl + cerr
#include <list>         // for std::list
#include <netinet/in.h> // for sockaddr_in
#include <queue>
#include <set>          // for storing sockets
#include <sstream>      // for stream()
#include <sys/socket.h> // for socket, listen, send
#include <unistd.h>     // for close

// importing helper files

#include "Client.h"
#include "ServerSettings.h"
#include "clientCommands.h"
#include "createSocket.h"
#include "ip.h"
#include "sendMessage.h"
#include "serverCommands.h"
#include "serverConnect.h"

ServerSettings groupSixServer;

std::set<Client *> unknownServers; // Set for for unknownServers
std::set<Client *> servers;        // Lookup table for servers
std::set<Client *> clients;        // Lookup table for clients
std::set<Client *> newServers;     // servers which have been newly connected

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

std::vector<std::string> splitMessages(const std::string &buffer) {
    std::vector<std::string> messages;
    size_t start = 0, end = 0;

    while (start < buffer.size()) {
        // Find the beginning and the end of the message
        start = buffer.find((char)0x02, start);
        end = buffer.find((char)0x03, start);

        if (start == std::string::npos || end == std::string::npos) {
            // Error: Incomplete message in buffer
            std::cerr << "Error: Incomplete message found." << std::endl;
            break; // Break out of loop if we don't find a valid STX or ETX
        }

        // Extract the message excluding STX and ETX and push to the vector
        messages.push_back(buffer.substr(start + 1, end - start - 1));

        // Move the start position past the current ETX for the next iteration
        start = end + 1;
    }

    return messages;
}

void clientCommand(int clientSocket, fd_set *openSockets, int *maxfds,
                   std::string message, int serverPort) {
    std::string invalidMessage = "ERROR";
    invalidMessage = (char)0x02 + invalidMessage + (char)0x03;

    if (message == "LISTSERVERS")
        return handleLISTSERVERS(clientSocket, servers);

    // under here, we should only have commands which must include comma's.
    size_t firstCommaIndex = message.find(',');

    if (firstCommaIndex == std::string::npos) {
        std::cout << "Invalid message received from (" << clientSocket
                  << "): " << message << std::endl;
        // send(clientSocket, invalidMessage.c_str(), invalidMessage.size(), 0);
        sendMessage(clientSocket, invalidMessage);
        return;
    }

    std::string command = message.substr(0, firstCommaIndex);
    std::string data = message.substr(firstCommaIndex + 1, message.size() - 1);

    if (command == "CONNECT") {
        Client *newClient = handleCONNECT(clientSocket, data, serverPort);
        if (newClient != nullptr)
            newServers.insert(newClient);
        return;
    }

    else if (command == "GETMSG")
        return handleGETMSG(clientSocket, data, groupSixServer);

    else if (command == "SENDMSG")
        return handleSENDMSG(clientSocket, data, servers, unknownServers,
                             groupSixServer);

    else
        return handleUNSUPPORTEDCLIENT(clientSocket, command);
}

void serverCommand(int serverSocket, fd_set *openSockets, int *maxfds,
                   std::string message, int serverPort) {
    std::string invalidMessage = "ERROR";
    invalidMessage = (char)0x02 + invalidMessage + (char)0x03;

    if (message == "ERROR")
        return handleERROR(serverSocket, message);

    size_t firstCommaIndex = message.find(',');

    if (firstCommaIndex == std::string::npos) {
        std::cout << "Invalid message received from (" << serverSocket
                  << "): " << message << std::endl;
        // send(serverSocket, invalidMessage.c_str(), invalidMessage.size(), 0);
        sendMessage(serverSocket, invalidMessage);
        return;
    }

    std::string command = message.substr(0, firstCommaIndex);
    std::string data = message.substr(firstCommaIndex + 1, message.size() - 1);

    if (command == "SERVERS")
        return handleSERVERS(serverSocket, data, servers);

    if (command == "KEEPALIVE")
        return handleKEEPALIVE(serverSocket, data, servers, groupSixServer);

    else if (command == "QUERYSERVERS")
        return handleQUERYSERVERS(serverSocket, data, servers, serverPort);

    else if (command == "FETCH_MSGS")
        return handleFETCH_MSGS(serverSocket, data, servers);

    else if (command == "SEND_MSG")
        return handleSEND_MSG(serverSocket, data, servers, unknownServers,
                              groupSixServer);

    else if (command == "STATUSREQ")
        return handleSTATUSREQ(serverSocket, data, servers, groupSixServer);

    else if (command == "STATUSRESP")
        return handleSTATUSRESP(serverSocket, data, servers, groupSixServer);

    else
        return handleUNSUPPORTED(serverSocket, command, data);
}

void acceptConnection(int socket, sockaddr_in socketAddress,
                      fd_set *openSockets, int *maxfds, ClientType clientType,
                      int serverPort) {
    socklen_t clientLen = sizeof(socketAddress);

    int newSocketConnection =
        accept(socket, (struct sockaddr *)&socketAddress, &clientLen);
    if (newSocketConnection == -1) {
        std::cerr << "Failed to accept client." << std::endl;
        return;
    }

    // Retrieve the IP address and port
    std::string clientIP = inet_ntoa(socketAddress.sin_addr);

    Client *newClient =
        new Client(newSocketConnection, clientType, clientIP, -1);

    if (clientType == ClientType::SERVER) {
        if (servers.size() >= groupSixServer.maxConnections) {
            // our server has reached max capacity, disconnect from server
            std::cout << "declined incoming connection from " << clientIP
                      << ". MAX connections hit" << std::endl;
            close(newSocketConnection);
            return;
        }
        servers.insert(newClient);
        sendQUERYSERVERS(serverPort, newSocketConnection);
    } else
        clients.insert(newClient);

    // Add new client to the list of open sockets
    FD_SET(newSocketConnection, openSockets);
    *maxfds = std::max(*maxfds, newSocketConnection);

    std::cout << newClient->clientTypeToString() << " " << newSocketConnection
              << " connected from " << clientIP << std::endl;
};

void handleClientMessage(Client *const &client, char *buffer, int bufferSize,
                         fd_set *openSockets,
                         std::list<Client *> &disconnectedClients, int *maxfds,
                         int *basemaxfds, int serverPort) {
    if (recv(client->sock, buffer, bufferSize, MSG_DONTWAIT) == 0) {
        disconnectedClients.push_back(client);
        closeClient(client, openSockets, maxfds, basemaxfds);
        return;
    }

    std::string newBuffer(buffer);
    std::vector<std::string> messages = splitMessages(newBuffer);

    if (messages.size() > 1)
        std::cout << "received " << messages.size() << " messages from client "
                  << client->sock << std::endl;

    for (const auto &message : messages)
        clientCommand(client->sock, openSockets, maxfds, message, serverPort);
};

void handleServerMessage(Client *const &server, char *buffer, int bufferSize,
                         fd_set *openSockets,
                         std::list<Client *> &disconnectedServers, int *maxfds,
                         int *basemaxfds, int serverPort) {
    // receive the message from the server
    if (recv(server->sock, buffer, bufferSize, MSG_DONTWAIT) == 0) {
        disconnectedServers.push_back(server);
        closeClient(server, openSockets, maxfds, basemaxfds);
        return;
    }
    std::string newBuffer(buffer);
    std::vector<std::string> messages = splitMessages(newBuffer);

    if (messages.size() > 1)
        std::cout << "received " << messages.size() << " messages from server "
                  << server->sock << std::endl;

    for (const auto &message : messages)
        serverCommand(server->sock, openSockets, maxfds, message, serverPort);
};

int main(int argc, char *argv[]) {
    fd_set openSockets, readSockets,
        exceptSockets; // open, listed, and exception sockets.
    int basemaxfds, maxfds, socketsReady, serverSocket, clientSocket;
    char buffer[1025]; // buffer for reading from clients
    auto start = std::chrono::steady_clock::now();
    groupSixServer.serverName = "P3_GROUP_6";

    if (argc != 3) {
        printf("Usage: ./server <server port> <client port>\n");
        exit(0);
    }

    int serverPort = atoi(argv[1]);
    int clientPort = atoi(argv[2]);
    struct sockaddr_in server_addr, client_addr;

    // Create socket
    serverSocket = createListenSocket(serverPort, server_addr);
    clientSocket = createListenSocket(clientPort, client_addr);

    std::cout << "Server listening at ip " << getMyIp() << " on port "
              << serverPort << " for servers and port " << clientPort
              << " for clients..." << std::endl;

    FD_ZERO(&openSockets);
    FD_SET(serverSocket, &openSockets);
    FD_SET(clientSocket, &openSockets);
    basemaxfds = maxfds = std::max(serverSocket, clientSocket);

    bool finished = false;
    while (!finished) {
        // Get modifiable copy of readSockets
        readSockets = exceptSockets = openSockets;
        memset(buffer, 0, sizeof(buffer)); // Initialize the buffer
        auto now = std::chrono::steady_clock::now();

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
                             ClientType::CLIENT, serverPort);

        if (FD_ISSET(serverSocket,
                     &readSockets)) // we have a new server connection
            acceptConnection(serverSocket, server_addr, &openSockets, &maxfds,
                             ClientType::SERVER, serverPort);

        // Now check for commands from clients
        std::list<Client *> disconnectedClients;
        std::list<Client *> disconnectedServers;

        for (auto const &client : clients)
            if (FD_ISSET(client->sock, &readSockets))
                handleClientMessage(client, buffer, sizeof(buffer),
                                    &openSockets, disconnectedClients, &maxfds,
                                    &basemaxfds, serverPort);

        for (Client *server : servers) {
            auto it = std::find_if(
                unknownServers.begin(), unknownServers.end(),
                [server](const Client *unknownServer) {
                    return *server == *unknownServer; // Assumes operator== is
                                                      // defined for Client
                });

            if (it != unknownServers.end()) {
                Client *unknownServer = *it;
                server->messages = unknownServer->messages;
                unknownServers.erase(it);
            }
        }

        // Remove client from the clients list
        for (auto const &c : disconnectedClients)
            clients.erase(c);

        for (auto const &server : servers)
            if (FD_ISSET(server->sock, &readSockets))
                handleServerMessage(server, buffer, sizeof(buffer),
                                    &openSockets, disconnectedServers, &maxfds,
                                    &basemaxfds, serverPort);

        // Remove client from the clients list
        for (auto const &s : disconnectedServers)
            servers.erase(s);

        // add servers created during this execution cycle to the set of
        // servers.
        for (auto const &newS : newServers) {
            std::cout << "adding server " << newS->toString()
                      << " to servers set" << std::endl;
            FD_SET(newS->sock, &openSockets);
            servers.insert(newS);
            maxfds = std::max(maxfds, newS->sock);
        }
        newServers.clear();

        auto elapsed = now - start;
        if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() >
            120) {
            sendKEEPALIVE(servers);
            start = std::chrono::steady_clock::now();
        }
    }

    // Close sockets
    close(serverSocket);
    close(clientSocket);

    return 0;
}
