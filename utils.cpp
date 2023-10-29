#include <string>
#include <vector>

bool isConvertibleToInt(std::string &str, int &result) {
    try {
        result = std::stoi(str);
        return true;
    } catch (const std::invalid_argument &) {
        // if no conversion could be performed
        return false;
    } catch (const std::out_of_range &) {
        // if the converted value would fall out of the range of the result type
        return false;
    }
    // for any other unexpected exceptions, return false
    catch (...) {
        return false;
    }
}

std::vector<std::string> splitString(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = s.find(delimiter);

    while (end != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + 1;
        end = s.find(delimiter, start);
    }

    // Handle the case where there's no trailing delimiter
    if (start != s.length()) {
        tokens.push_back(s.substr(start));
    }

    return tokens;
}
