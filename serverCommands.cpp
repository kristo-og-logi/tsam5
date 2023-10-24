#include <iostream> // for std::cout + endl
#include <set>      // set
#include <sstream>
#include <string>       // for std::string
#include <sys/socket.h> // for socket, listen, send
#include <vector>

#include "Client.h"
#include "serverCommands.h"

const std::string GROUP_NAME = "P3_GROUP_6";
const std::string MY_IP = "130.208.243.61";

void handleKEEPALIVE(int socket, const std::string data) {
    std::cout << "keepalive: " << data << std::endl;

    std::string response = "KEEPALIVE, P3_GROUP_6\n";
    send(socket, response.c_str(), response.size(), 0);

    return;
}

void handleQUERYSERVERS(int socket, const std::string data,
                        const std::set<Client *> &servers, int serverPort) {
    std::cout << "QUERYSERVERS sent with data: " << data << std::endl;

    std::string response = "SERVERS," + GROUP_NAME + "," + MY_IP + "," +
                           std::to_string(serverPort) + ";";

    for (Client *server : servers) {
        if (server->sock == socket)
            server->name = data;
        response += server->toString();
    }

    response += "\n";

    send(socket, response.c_str(), response.size(), 0);
    return;
}

void handleFETCH_MSGS(int socket, const std::string data) {
    std::cout << "FETCH_MSGS: " << data << std::endl;

    std::string response = "FETCH_MSGS, P3_GROUP_6\n";
    send(socket, response.c_str(), response.size(), 0);

    return;
}

std::vector<unsigned char>
constructMessage(std::vector<std::string> command_msg) {
    unsigned char prefix = 0x02;
    unsigned char postfix = 0x03;
    std::vector<unsigned char> buffer;

    buffer.push_back(prefix);

    for (size_t i = 0; i < command_msg.size(); i++) {
        if (i > 0) {
            buffer.push_back(',');
        }
        buffer.insert(buffer.end(), command_msg[i].begin(),
                      command_msg[i].end());
    }

    buffer.push_back('\n');
    buffer.push_back(postfix);

    return buffer;
}

void handleSEND_MSG(int socket, const std::string data,
                    const std::set<Client *> &servers) {
    unsigned char prefix = 0x02;
    unsigned char postfix = 0x03;
    int messageSent = 0;

    std::string toGroup, fromGroup, content;
    std::string command = "SEND_MSG";

    std::stringstream ss(data);
    std::getline(ss, toGroup, ',');
    std::getline(ss, fromGroup, ',');
    std::getline(ss, content);

    std::vector<std::string> command_msg = {command, toGroup, fromGroup,
                                            content};

    for (Client *server : servers) {
        if (server->name == toGroup) {
            std::vector<unsigned char> buffer = constructMessage(command_msg);
            std::vector<unsigned char> success =
                constructMessage({"Successfully sent message"});

            send(server->sock, buffer.data(), buffer.size(), 0);
            send(socket, success.data(), success.size(), 0);
            messageSent = 1;
        }
    }

    if (messageSent == 0) {
        std::vector<unsigned char> response =
            constructMessage({"Could not send message"});
        send(socket, response.data(), response.size(), 0);
    }

    return;
}

void handleSTATUSREQ(int socket, const std::string data) {
    std::cout << "STATUSREQ: " << data << std::endl;

    std::string response = "STATUSREQ, P3_GROUP_6\n";
    send(socket, response.c_str(), response.size(), 0);

    return;
}

void handleSTATUSRESP(int socket, const std::string data) {
    std::cout << "STATUSRESP: " << data << std::endl;

    std::string response = "STATUSRESP, P3_GROUP_6\n";
    send(socket, response.c_str(), response.size(), 0);

    return;
}

void handleUNSUPPORTED(int socket, const std::string command,
                       const std::string data) {
    std::cout << "unsupported server command: " << command << " received"
              << std::endl;

    std::string response = "UNSUPPORTED, P3_GROUP_6\n";
    send(socket, response.c_str(), response.size(), 0);

    return;
}
