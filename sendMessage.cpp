#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <sys/socket.h>

void sendMessage(int socket, std::string message) {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

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
        std::cout << std::put_time(std::localtime(&currentTime),
                                   "%Y-%m-%d %H:%M:%S") << " | "
                  << socket << " | Sent: " << message << std::endl;
}
