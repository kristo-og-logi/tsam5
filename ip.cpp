#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#include <iostream>

std::string getEn0IPAddress(const std::string &data) {
    std::istringstream stream(data);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.substr(0, 3) == "en0") {
            std::size_t pos = line.find(": ");
            if (pos != std::string::npos) {
                std::string ip = line.substr(pos + 2);
                // Filter out non-IPv4 addresses (e.g., IPv6 addresses contain
                // colons)
                if (ip.find(":") == std::string::npos) {
                    return ip;
                }
            }
        }
    }
    return "Not Found";
}

std::string getMyIp() {
    struct ifaddrs *myaddrs, *ifa;
    void *in_addr;
    char buf[64];

    if (getifaddrs(&myaddrs) != 0) {
        perror("getifaddrs");
        exit(1);
    }
    std::string output;

    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;

        switch (ifa->ifa_addr->sa_family) {
        case AF_INET: {
            struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
            in_addr = &s4->sin_addr;
            break;
        }

        case AF_INET6: {
            struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
            in_addr = &s6->sin6_addr;
            break;
        }

        default:
            continue;
        }

        if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf))) {
            printf("%s: inet_ntop failed!\n", ifa->ifa_name);
        } else {
            output += ifa->ifa_name;
			output += ": ";
			output += std::string(buf) + "\n";
            // printf("%s: %s\n", ifa->ifa_name, buf);
        }
    }

    freeifaddrs(myaddrs);
    return getEn0IPAddress(output);
}
