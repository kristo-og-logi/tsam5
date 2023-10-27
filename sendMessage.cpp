#include <iostream>
#include <string>
#include <sys/socket.h>

void sendMessage(int socket, std::string message) {
    if (message.front() != 0x02)
        message = '\x02' + message;

    if (message.back() != 0x03)
        message += '\x03';

    if (message.size() > 5000) {
        std::cerr << "ERROR: sending too long message (" << socket
                  << "): " << message.substr(0, 100) << "..." << std::endl;
        return;
    }

    int bytesSent = send(socket, message.c_str(), message.size(), 0);
    if (bytesSent < 0)
        std::cerr << "Error sending (" << socket << "): " << message
                  << std::endl;

    else if (bytesSent == 0)
        // We're not going to disconnect the server here, as it makes the code
        // unbelievably more ugly to pipe the disconnectedServers all the way
        // here and it's very unlikely. Rather, we'll let the main function deal
        // with disconnecting.
        std::cout << "CHECK THIS: tried sending data to a closed connection ("
                  << socket << ")" << std::endl;
    else
        std::cout << "Sent (" << socket << "): " << message << std::endl;
}
