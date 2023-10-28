// clientCommands.h

#ifndef CLIENT_COMMANDS_H
#define CLIENT_COMMANDS_H

#include <set>
#include <string>

#include "Client.h"
#include "ServerSettings.h"

void handleLISTSERVERS(int socket, std::set<Client *> &servers,
                       ServerSettings &groupSixServer);

Client *handleCONNECT(int socket, std::string data, int serverPort,
                      ServerSettings &myServer);

void handleGETMSG(int socket, const std::string data, ServerSettings &myServer);

void handleSENDMSG(int socket, std::string data, std::set<Client *> &servers,
                   std::set<Client *> &unknownServers,
                   ServerSettings &myServer);

void handleUNSUPPORTEDCLIENT(int socket, std::string command);

#endif // CLIENT_COMMANDS_H
