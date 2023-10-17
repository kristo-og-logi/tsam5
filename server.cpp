#include <algorithm>
#include <arpa/inet.h>
#include <errno.h>
#include <list>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include <iostream>
#include <map>
#include <sstream>
#include <thread>

#include <unistd.h>

// fix SOCK_NONBLOCK for OSX
#ifndef SOCK_NONBLOCK
#include <fcntl.h>
#define SOCK_NONBLOCK O_NONBLOCK
#endif

#define BACKLOG 5 // Allowed length of queue of waiting connections

std::mutex serverThreadsMutex;
std::condition_variable serverThreadsCondition;
int activeServerThreads = 0;
const int maxServerThreads = 10;

typedef struct {
  int sock;
  std::string name;
} Client;

std::map<int, Client> clients;
std::map<int, Client> servers;

int open_socket(int portno) {
  struct sockaddr_in sk_addr; // address settings for bind()
  int sock;                   // socket opened for this port
  int set = 1;                // for setsockopt

  // Create socket for connection. Set to be non-blocking, so recv will
  // return immediately if there isn't anything waiting to be read.
#ifdef __APPLE__
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Failed to open socket");
    return (-1);
  }
#else
  if ((sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
    perror("Failed to open socket");
    return (-1);
  }
#endif

  // Turn on SO_REUSEADDR to allow socket to be quickly reused after
  // program exit.

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set)) < 0) {
    perror("Failed to set SO_REUSEADDR:");
  }
  set = 1;
#ifdef __APPLE__
  if (setsockopt(sock, SOL_SOCKET, SOCK_NONBLOCK, &set, sizeof(set)) < 0) {
    perror("Failed to set SOCK_NOBBLOCK");
  }
#endif
  memset(&sk_addr, 0, sizeof(sk_addr));

  sk_addr.sin_family = AF_INET;
  sk_addr.sin_addr.s_addr = INADDR_ANY;
  sk_addr.sin_port = htons(portno);

  // Bind to socket to listen for connections from clients

  if (bind(sock, (struct sockaddr *)&sk_addr, sizeof(sk_addr)) < 0) {
    perror("Failed to bind to socket:");
    return (-1);
  }

  if (listen(sock, SOMAXCONN) <
      0) { // SOMAXCONN is the maximum number of queued connections
    std::cerr << "Listen failed!" << std::endl;
    close(sock);
    return -1;
  }

  return (sock);
}

/*
 * @description Close client that is connected to the server
 * @param int clientsocketfd
 */
void closeClient(int clientSocket) {}

/*
 * @description Server handling of client commands
 * @param int socketfd
 * @param char * buffer
 */
void clientCommand(int clientsocket, char *buffer) {}

/*
 * @description Server handling other server commands
 * @param int sockfd
 * @param char * buffer
 */
void serverCommand(int clientsocket, char *buffer) {}

/*
 * @description Handle server communications, encapsulates server to server
 * logic
 * @param int socketfd
 */
int handleServer(int sock) {
  std::cout << "This is server:" << sock << std::endl;
  return 0;
}

/*
 * @description Handle client communications with the server, encapsulates
 * client to server logic
 * @param int socketfd
 */
int handleClient(int sock) {
  std::cout << "This is client:" << sock << std::endl;
  return 0;
}

/*
 * @description Accept incoming server connection requests
 * @param int listensocket
 */
int acceptServerConnection(int listenSocket) {
  struct sockaddr_in serverAddress;
  socklen_t serverLen = sizeof(serverAddress);

  int serverSocket =
      accept(listenSocket, (struct sockaddr *)&serverAddress, &serverLen);

  if (serverSocket < 0) {
    // std::cerr << "Failed to accept connection" << std::endl;
    return -1;
  }
  std::cout << "Connection accepted" << std::endl;


  Client newServer;
  newServer.sock = serverSocket;
  newServer.name = "Server" + std::to_string(serverSocket);

  servers[serverSocket] = newServer;
  
  return serverSocket;
}

/*
 * @description Accept incoming client connection requests
 * @param int listensocket
 */
int acceptClientConnection(int listenSocket) {
  struct sockaddr_in clientAddress;
  socklen_t clientLen = sizeof(clientAddress);

  int clientSocket =
      accept(listenSocket, (struct sockaddr *)&clientAddress, &clientLen);

  if (clientSocket < 0) {
    // std::cerr << "Failed to accept connection" << std::endl;
    return -1;
  }
  std::cout << "Connection accepted" << std::endl;

  Client newClient;
  newClient.sock = clientSocket;
  newClient.name = "Server" + std::to_string(clientSocket);

  servers[clientSocket] = newClient;

  return clientSocket;
}

int main(int argc, char *argv[]) {
  int serverSocket = open_socket(4044);
  int clientSocket = open_socket(4022);

  // Server thread
  std::thread serverThread([&]() {
    while (true) {
      int acceptedServerSocket = acceptServerConnection(serverSocket);
      if (acceptedServerSocket > 0) {
        std::unique_lock<std::mutex> lock(serverThreadsMutex);

        // Wait if we've reached the maximum number of threads.
        serverThreadsCondition.wait(
            lock, []() { return activeServerThreads < maxServerThreads; });

        ++activeServerThreads;
        lock.unlock();

        std::thread handleServerThread(handleServer, acceptedServerSocket);
        handleServerThread.detach();
      }
    }
  });

  // Client thread
  std::thread clientThread([&]() {
    while (true) {
      int acceptedClientSocket = acceptClientConnection(clientSocket);
      if (acceptedClientSocket > 0) {
        std::thread handleClientThread(handleClient, acceptedClientSocket);
        handleClientThread.detach();
      }
    }
  });

  // Join threads
  serverThread.join();
  clientThread.join();
  return 0;
}
