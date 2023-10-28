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
    const int maxConnections = 10;

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

    int getMessageCount(std::string otherServerName) {
        auto it = serverMessages.find(otherServerName);
        if (it != serverMessages.end()) {
            return it->second.size();
        }
        return 0;
    }
    void eraseServer(std::string otherServer) {
        serverMessages.erase(otherServer);
    }
};

#endif // SERVER_SETTINGS_H
