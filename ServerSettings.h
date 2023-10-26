#ifndef SERVER_SETTINGS_H
#define SERVER_SETTINGS_H

#include <queue>
#include <string>

class ServerSettings {
  private:
    std::queue<std::string> serverMessages;

  public:
    std::string serverName;
    std::string ipAddr;

    void addMessage(const std::string &message) {
        serverMessages.push(message);
    }

    std::string getMessage() {
        if (!serverMessages.empty()) {
            std::string message = serverMessages.front();
            serverMessages.pop();
            return message;
        }
        return "";
    }
};

#endif // SERVER_SETTINGS_H
