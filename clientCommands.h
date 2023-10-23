// clientCommands.h

#ifndef CLIENT_COMMANDS_H
#define CLIENT_COMMANDS_H

#include <string>

void handleLISTSERVERS(int socket, std::set<Client *> &servers);

#endif // CLIENT_COMMANDS_H
