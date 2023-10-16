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

typedef struct {
  int sock;
  std::string name;
} Client;

std::map<int, Client *> clients;

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
  } else {
    return (sock);
  }
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
int handleServer(int sock) { return 0; }

/*
 * @description Handle client communications with the server, encapsulates
 * client to server logic
 * @param int socketfd
 */
int handleClient(int sock) { return 0; }

/*
 * @description Accept incoming server connection requests
 * @param int listensocket
 */
int acceptServerConnection(int listenSocket) { return 0; }

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
    std::cerr << "Failed to accept connection" << std::endl;
    return -1;
  }
  std::cout << "Connection accepted" << std::endl;
  return clientSocket;
}

int main(int argc, char *argv[]) {
  // Server thread

  // Client thread

  // Join threads
  return 0;
}
