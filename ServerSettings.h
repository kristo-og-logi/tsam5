#ifndef SERVER_SETTINGS_H
#define SERVER_SETTINGS_H

#include <chrono>
#include <iomanip>
#include <map>
#include <queue>
#include <sstream>
#include <string>

class ServerSettings {
  private:
    std::map<std::string,
             std::queue<
                 std::pair<std::chrono::system_clock::time_point, std::string>>>
        serverMessages;

  public:
    std::string serverName;
    std::string ipAddr;
    const int maxConnections = 10;

    void addMessage(std::string otherServer, const std::string &message) {
        auto now = std::chrono::system_clock::now();
        serverMessages[otherServer].emplace(now, message);
    }

    std::string getMessage(std::string otherServer) {
        if (serverMessages.count(otherServer) > 0) {
            if (!serverMessages[otherServer].empty()) {
                auto unformatted_msg = serverMessages[otherServer].front();
                serverMessages[otherServer].pop();
                auto timestamp = unformatted_msg.first;
                std::string message = unformatted_msg.second;

                std::time_t time =
                    std::chrono::system_clock::to_time_t(timestamp);
                std::tm tm = *std::localtime(&time);
                std::stringstream ss;
                ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " | " << otherServer + ": "
                   << message;

                return ss.str();
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
