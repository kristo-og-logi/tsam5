#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

// fix SOCK_NONBLOCK for OSX
#ifndef SOCK_NONBLOCK
#include <fcntl.h>
#define SOCK_NONBLOCK O_NONBLOCK
#endif

int createSocket(int portno, struct sockaddr_in addr) {
    socklen_t addr_len = sizeof(addr);
    int sock;

#ifdef __APPLE__
    sock = socket(AF_INET, SOCK_STREAM, 0);
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
        perror("Failed to set SO_REUSEPORT:");
    }

#ifdef __APPLE__
    set = 1;
    if (setsockopt(sock, SOL_SOCKET, SOCK_NONBLOCK, &set, sizeof(set)) < 0) {
        perror("Failed to set SOCK_NONBLOCK");
    }
#endif

    return sock;
}

int createListenSocket(int listenPort, struct sockaddr_in addr) {
    int sock = createSocket(listenPort, addr);

    // Setup server address structure
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(listenPort);

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

int createConnection(std::string outIp, int outPort, struct sockaddr_in addr) {
    int sock = createSocket(outPort, addr);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(outPort);
    addr.sin_addr.s_addr = inet_addr(outIp.c_str());

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        if (errno != EINPROGRESS) {
            std::cerr << "Failed to connect: " << std::strerror(errno)
                      << std::endl;
            close(sock);
            return -1;
        }
    }

    std::cout << "Connected to server " << outIp << ":" << outPort << std::endl;

    return sock;
}
