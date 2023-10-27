#ifndef CREATE_SOCKET_H
#define CREATE_SOCKET_H

#include <netinet/in.h>
#include <string>

int createSocket(int portno, struct sockaddr_in addr);

int createListenSocket(int portno, struct sockaddr_in addr);

int createConnection(std::string outIp, int outPort, struct sockaddr_in addr);

#endif // CREATE_SOCKET_H
