#ifndef SERVER_CONNECT_H
#define SERVER_CONNECT_H

#include <arpa/inet.h> // For inet_addr()
#include <cstring>     // For std::strerror()
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <string>
#include <sys/socket.h> // For socket(), connect()
#include <unistd.h>     // For close()

#include "Client.h"

Client *connectToServer(std::string &data);

#endif // SERVER_CONNECT_H
