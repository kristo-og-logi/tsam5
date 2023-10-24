#ifndef SERVER_CONNECT_H
#define SERVER_CONNECT_H

#include <iostream>
#include <cstring>      // For std::strerror()
#include <sys/socket.h> // For socket(), connect()
#include <netinet/in.h> // For sockaddr_in
#include <arpa/inet.h>  // For inet_addr()
#include <unistd.h>     // For close()
#include <string>

int connectToServer(const std::string& data);

#endif // SERVER_CONNECT_H

