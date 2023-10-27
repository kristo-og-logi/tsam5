#ifndef SERVER_SETTINGS_H
#define SERVER_SETTINGS_H

#include <map>
#include <queue>
#include <string>

class ServerSettings {
  private:
    std::map<std::string, std::queue<std::string>> serverMessages;

  public:
    std::string serverName;
    std::string ipAddr;

    void addMessage(std::string otherServer, const std::string &message) {
        serverMessages[otherServer].push(message);
    }

    std::string getMessage(std::string otherServer) {
        if (serverMessages.count(otherServer) > 0) {
            if (!serverMessages[otherServer].empty()) {
                std::string message = serverMessages[otherServer].front();
                serverMessages[otherServer].pop();
                return message;
            }
        }
        return "";
    }
};

#endif // SERVER_SETTINGS_H
