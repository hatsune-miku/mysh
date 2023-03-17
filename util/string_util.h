#pragma once

#include <string>
#include <vector>
#include <string_view>

std::vector<std::string> string_split(const std::string &str, std::string_view delim);

std::string string_join(const std::vector<std::string> &strs, std::string_view delim);

std::string string_color(std::string_view str, int color);
std::string string_color(std::string_view str, int color, int limit);

std::string_view string_strip(std::string_view str);
