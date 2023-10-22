#include <algorithm>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include <iostream>
#include <sstream>

// Threaded function for handling responss from server

void listenServer(int serverSocket) {
    int nread;         // Bytes read from socket
    char buffer[1025]; // Buffer for reading input

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        nread = read(serverSocket, buffer, sizeof(buffer));

        if (nread == 0) // Server has dropped us
        {
            printf("Connection closed\n");
            exit(0);
        } else if (nread > 0)
            printf("%s", buffer);
    }
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *svr;  // Network host entry for server
    struct sockaddr_in serv_addr; // Socket address for server
    int serverSocket;             // Socket used for server
    int nwrite;                   // No. bytes written to server
    char buffer[1025];            // buffer for writing to server
    bool finished;
    int set = 1; // Toggle for setsockopt

    if (argc != 3) {
        printf("Usage: ./client <ip> <port>\n");
        exit(0);
    }

    hints.ai_family = AF_INET; // IPv4 only addresses
    hints.ai_socktype = SOCK_STREAM;

    memset(&hints, 0, sizeof(hints));

    if (getaddrinfo(argv[1], argv[2], &hints, &svr) != 0) {
        perror("getaddrinfo failed: ");
        exit(0);
    }

    struct hostent *server;
    server = gethostbyname(argv[1]);

    memset((char *)&serv_addr, 0,
           sizeof(serv_addr)); // clear the server address info

    // more robust version of:
    // serv_addr.sin_addr.s_addr = *(in_addr_t *)server->h_addr;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Failed to create socket");
        exit(0);
    }

    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set)) <
        0) {
        printf("Failed to set SO_REUSEADDR for port %s\n", argv[2]);
        perror("setsockopt failed: ");
    }

    if (connect(serverSocket, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0) {
        // EINPROGRESS means that the connection is still being setup. Typically
        // this only occurs with non-blocking sockets. (The serverSocket above
        // is explicitly not in non-blocking mode, so this check here is just an
        // example of how to handle this properly.)
        if (errno != EINPROGRESS) {
            printf("Failed to open socket to server: %s\n", argv[1]);
            perror("Connect failed: ");
            exit(0);
        }
    }

    // Listen and print replies from server
    std::thread serverThread(listenServer, serverSocket);

    finished = false;
    while (!finished) {
        bzero(buffer, sizeof(buffer)); // clean the buffer

        fgets(buffer, sizeof(buffer), stdin); // read a line from stdin

        if ((nwrite = send(serverSocket, buffer, strlen(buffer), 0)) == -1) {
            perror("send() to server failed: ");
            finished = true;
        }
    }
}
