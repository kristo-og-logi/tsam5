// clientCommands.h

#ifndef CLIENT_COMMANDS_H
#define CLIENT_COMMANDS_H

#include <string>

void handleLISTSERVERS(int socket, std::set<Client *> &servers);

void handleGETMSG(int socket);

void handleSENDMSG(int socket);

void handleCONNECT(int socket);

void handleUNSUPPORTEDCLIENT(int socket, std::string command);

#endif // CLIENT_COMMANDS_H
