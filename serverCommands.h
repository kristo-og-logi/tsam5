// serverCommands.h

#ifndef SERVER_COMMANDS_H
#define SERVER_COMMANDS_H

#include "Client.h"
#include "ServerSettings.h"
#include <queue>
#include <set>
#include <string>

void handleSERVERS(int socket, const std::string data);

void sendKEEPALIVE(std::set<Client *> servers);

void handleKEEPALIVE(int socket, const std::string data,
                     const std::set<Client *> &servers,
                     ServerSettings &myServer);

void handleQUERYSERVERS(int socket, const std::string data,
                        const std::set<Client *> &servers, int serverPort);

void handleFETCH_MSGS(int socket, const std::string data,
                      const std::set<Client *> &servers);

void handleSEND_MSG(int socket, const std::string data,
                    const std::set<Client *> &servers,
                    std::set<Client *> &unknownServers,
                    ServerSettings &myServer);

void handleSTATUSREQ(int socket, const std::string data,
                     const std::set<Client *> &servers,
                     ServerSettings &myServer);

void handleSTATUSRESP(int socket, const std::string data,
                      const std::set<Client *> &servers,
                      ServerSettings &myServer);

void handleUNSUPPORTED(int socket, const std::string command,
                       const std::string data);

#endif // SERVER_COMMANDS_H
