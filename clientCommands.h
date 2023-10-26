// clientCommands.h

#ifndef CLIENT_COMMANDS_H
#define CLIENT_COMMANDS_H

#include <set>
#include <string>

#include "Client.h"

void handleLISTSERVERS(int socket, std::set<Client *> &servers);

Client *handleCONNECT(int socket, std::string data, int serverPort);

void handleGETMSG(int socket);

void handleSENDMSG(int socket);

void handleUNSUPPORTEDCLIENT(int socket, std::string command);

#endif // CLIENT_COMMANDS_H
