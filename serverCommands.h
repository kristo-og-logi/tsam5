// serverCommands.h

#ifndef SERVER_COMMANDS_H
#define SERVER_COMMANDS_H

#include <string>

void handleKEEPALIVE(int socket, const std::string data);

void handleQUERYSERVERS(int socket, const std::string data,
                        const std::set<Client *> &servers);

void handleFETCH_MSGS(int socket, const std::string data);

void handleSEND_MSG(int socket, const std::string data);

void handleSTATUSREQ(int socket, const std::string data);

void handleSTATUSRESP(int socket, const std::string data);

void handleUNSUPPORTED(int socket, const std::string command,
                       const std::string data);

#endif // SERVER_COMMANDS_H
