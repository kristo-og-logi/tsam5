#ifndef IP_H
#define IP_H

#include <string>

// Queries the system to get IP addresses for all network interfaces.
// Then extracts the IP address associated with "en0".
// Returns the IP address if found, otherwise returns "Not Found".
std::string getEn0IPAddress(const std::string &data);
std::string getMyIp();

#endif // IP_H

