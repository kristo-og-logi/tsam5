#include <cstring>
#include <iostream>     // for std::cout + endl + cerr
#include <list>         // for std::list
#include <map>          // for std::map
#include <netinet/in.h> // for sockaddr_in
#include <sstream>      // for stream()
#include <sys/socket.h> // for socket, listen
#include <unistd.h>     // for close

// fix SOCK_NONBLOCK for OSX
#ifndef SOCK_NONBLOCK
#include <fcntl.h>
#define SOCK_NONBLOCK O_NONBLOCK
#endif

// Simple class for handling connections from clients.
//
// Client(int socket) - socket to send/receive traffic from client.
class Client {
  public:
    int sock;         // socket of client connection
    std::string name; // Limit length of name of client's user

    Client(int socket) : sock(socket) {}

    ~Client() {} // Virtual destructor defined for base class
};

// Note: map is not necessarily the most efficient method to use here,
// especially for a server with large numbers of simulataneous connections,
// where performance is also expected to be an issue.
//
// Quite often a simple array can be used as a lookup table,
// (indexed on socket no.) sacrificing memory for speed.

std::map<int, Client *> clients; // Lookup table for per Client information

int createSocket(int portno, struct sockaddr_in addr) {
    socklen_t addr_len = sizeof(addr);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }

    int set = 1; // for setsockopt
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set)) < 0) {
        perror("Failed to set SO_REUSEADDR:");
    }
    set = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &set, sizeof(set)) < 0) {
        perror("Failed to set SO_REUSEADDR:");
    }
    set = 1;
    if (setsockopt(sock, SOL_SOCKET, SOCK_NONBLOCK, &set, sizeof(set)) < 0) {
        perror("Failed to set SOCK_NOBBLOCK");
    }

    // Setup server address structure
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(portno);

    // Bind socket
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        std::cerr << "Failed to bind socket." << std::endl;
        close(sock);
        return 1;
    }

    // Listen
    if (listen(sock, 5) == -1) {
        std::cerr << "Failed to listen on socket." << std::endl;
        close(sock);
        return 1;
    }

    return sock;
}

void closeClient(int clientSocket, fd_set *openSockets, int *maxfds) {
    printf("Client closed connection: %d\n", clientSocket);

    close(clientSocket);

    if (*maxfds == clientSocket) {
        for (auto const &p : clients) {
            *maxfds = std::max(*maxfds, p.second->sock);
        }
    }

    // And remove from the list of open sockets.

    FD_CLR(clientSocket, openSockets);
}

void clientCommand(int clientSocket, fd_set *openSockets, int *maxfds,
                   char *buffer) {
    std::vector<std::string> tokens;
    std::string token;

    // Split command from client into tokens for parsing
    std::stringstream stream(buffer);

    while (stream >> token)
        tokens.push_back(token);

    for (const auto &token : tokens) {
        std::cout << token << "-"; // or just " " instead of std::endl if
                                   // you want them on the same line
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[]) {
    bool finished;
    int listenSock;       // Socket for connections to server
    int clientSock;       // Socket of connecting client
    fd_set openSockets;   // Current open sockets
    fd_set readSockets;   // Socket list for select()
    fd_set exceptSockets; // Exception socket list
    int maxfds;           // Passed to select() as max fd in set
    struct sockaddr_in client;
    socklen_t clientLen;
    char buffer[1025]; // buffer for reading from clients

    if (argc != 3) {
        printf("Usage: chat_server <server port> <client port>\n");
        exit(0);
    }

    int serverPort = atoi(argv[1]);
    int clientPort = atoi(argv[2]);

    struct sockaddr_in server_addr, client_addr;

    int serverSocket, clientSocket;
    std::cout << "Starting a server" << std::endl;

    // Create socket
    serverSocket = createSocket(serverPort, server_addr);
    clientSocket = createSocket(clientPort, client_addr);

    std::cout << "Server listening on port " << serverPort
              << " for servers and port " << clientPort << " for clients..."
              << std::endl;

    FD_ZERO(&openSockets);
    FD_SET(serverSocket, &openSockets);
    FD_SET(clientSocket, &openSockets);
    maxfds = std::max(serverSocket, clientSocket);

    finished = false;

    while (!finished) {
        // Get modifiable copy of readSockets
        readSockets = exceptSockets = openSockets;
        memset(buffer, 0, sizeof(buffer)); // Initialize the buffer

        // Look at sockets and see which ones have something to be read()
        int n = select(maxfds + 1, &readSockets, NULL, &exceptSockets, NULL);

        if (n < 0) {
            perror("select failed - closing down\n");
            finished = true;
            break;
        }

        if (FD_ISSET(clientSocket, &readSockets)) {

            // Accept
            socklen_t clientLen = sizeof(client_addr);
            int newClientSock = accept(
                clientSocket, (struct sockaddr *)&client_addr, &clientLen);
            if (newClientSock == -1) {
                std::cerr << "Failed to accept client." << std::endl;
                continue; // Go back to listening for the next client
            }

            // Add new client to the list of open sockets
            FD_SET(newClientSock, &openSockets);

            // And update the maximum file descriptor
            maxfds = std::max(maxfds, newClientSock);

            // create a new client to store information.
            clients[newClientSock] = new Client(newClientSock);

            printf("Client connected on server: %d\n", newClientSock);

            // Send a message to the client
            const char *message = "Hello, Client!\n";
            send(newClientSock, message, strlen(message), 0);
        }
        if (FD_ISSET(serverSocket, &readSockets)) {

            // Accept
            socklen_t serverLen = sizeof(server_addr);
            int newServerSock = accept(
                serverSocket, (struct sockaddr *)&server_addr, &serverLen);
            if (newServerSock == -1) {
                std::cerr << "Failed to accept server." << std::endl;
                continue; // Go back to listening for the next client
            }

            // Add new client to the list of open sockets
            FD_SET(newServerSock, &openSockets);

            // And update the maximum file descriptor
            maxfds = std::max(maxfds, newServerSock);

            // create a new client to store information.
            clients[newServerSock] = new Client(newServerSock);

            printf("Server connected on server: %d\n", newServerSock);

            // Send a message to the client
            const char *message = "Hello, Server!\n";
            send(newServerSock, message, strlen(message), 0);
        }

        // Now check for commands from clients
        std::list<Client *> disconnectedClients;

        for (auto const &pair : clients) {
            Client *client = pair.second;

            if (FD_ISSET(client->sock, &readSockets)) {
                // recv() == 0 means client has closed connection
                if (recv(client->sock, buffer, sizeof(buffer), MSG_DONTWAIT) ==
                    0) {
                    disconnectedClients.push_back(client);
                    closeClient(client->sock, &openSockets, &maxfds);

                }
                // We don't check for -1 (nothing received) because select()
                // only triggers if there is something on the socket for us.
                else {
                    if (strncmp(buffer, "bye", 3) == 0) {
                        disconnectedClients.push_back(client);
                        closeClient(client->sock, &openSockets, &maxfds);
                    }

                    else
                        // std::cout << buffer << std::endl;
                        clientCommand(client->sock, &openSockets, &maxfds,
                                      buffer);
                }
            }
        }
        // Remove client from the clients list
        for (auto const &c : disconnectedClients)
            clients.erase(c->sock);

        // while (true) {

        //   ssize_t bytes_received =
        //       recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        //   if (bytes_received < 0) {
        //     std::cerr << "Failed to receive data from client." << std::endl;
        //   } else if (bytes_received == 0) {
        //     std::cout << "Client disconnected." << std::endl;
        //   } else {
        //     std::cout << "Received from client: " << buffer << std::endl;
        //   }

        //   if (strncmp(buffer, "bye", 3) == 0)
        //     break;
        // }

        // Close client socket
        // close(clientSocket);
    }

    // Close server socket (in practice, you'll likely never reach this point)
    close(serverSocket);
    close(clientSocket);

    return 0;
}
