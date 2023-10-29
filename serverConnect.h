#ifndef SERVER_CONNECT_H
#define SERVER_CONNECT_H

#include <arpa/inet.h> // For inet_addr()
#include <cstring>     // For std::strerror()
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <set>
#include <string>
#include <sys/socket.h> // For socket(), connect()
#include <unistd.h>     // For close()

#include "Client.h"
#include "ServerSettings.h"

void sendQUERYSERVERS(int serverPort, int sock, ServerSettings &myServer);

Client *connectToServer(std::string &data, int serverPort,
                        ServerSettings &myServer, std::set<Client *> &servers);

#endif // SERVER_CONNECT_H
