#include <string>

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
