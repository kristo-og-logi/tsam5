#include <iostream> // for std::cout + endl
#include <string>   // for std::string

#include "serverCommands.h"

void handleQUERYSERVERS(std::string data) { std::cout << data << std::endl; }

// int main() {
//     handleQUERYSERVERS("Test data");
//     return 0;
// }