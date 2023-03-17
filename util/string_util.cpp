#include "util/string_util.h"

#include <sstream>

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

std::vector<std::string> string_split(const std::string &str, std::string_view delim) {
    // Merge consecutive delimiters
    std::string merged;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == delim[0]) {
            merged += delim;
            while (str[i] == delim[0]) {
                i++;
            }
            i--;
        } else {
            merged += str[i];
        }
    }

    std::vector<std::string> result;
    size_t start = 0;
    size_t end = merged.find(delim);
    while (end != std::string::npos) {
        result.push_back(merged.substr(start, end - start));
        start = end + delim.length();
        end = merged.find(delim, start);
    }
    result.push_back(merged.substr(start, end));
    return result;
}

std::string string_join(const std::vector<std::string> &strs, std::string_view delim) {
    std::string result;
    for (size_t i = 0; i < strs.size(); i++) {
        result += strs[i];
        if (i != strs.size() - 1) {
            result += delim;
        }
    }
    return result;
}

std::string string_color(std::string_view str, int color) {
    return std::string("\033[")
        .append(std::to_string(color))
        .append("m")
        .append(str)
        .append("\033[0m");
}

std::string string_color(std::string_view str, int color, int limit) {
    std::string_view colored = str.substr(0, min(str.length(), limit));
    std::string_view rest = str.substr(min(str.length(), limit));

    return std::string("\033[")
        .append(std::to_string(color))
        .append("m")
        .append(colored)
        .append("\033[0m")
        .append(rest);
}

std::string_view string_strip(std::string_view str) {
    size_t start = 0;
    size_t end = str.length() - 1;
    while (str[start] == 32 || str[start] == 13 || str[start] == 10 || str[start] == 9) {
        ++start;
    }
    while (str[end] == 32 || str[end] == 13 || str[end] == 10 || str[end] == 9) {
        --end;
    }
    return str.substr(start, end - start + 1);
}
