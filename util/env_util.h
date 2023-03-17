//
// Created by Hatsune Miku on 2023-03-15.
//

#pragma once

#include <string>
#include <string_view>
#include <optional>

bool is_in_PATH(std::string_view file);

std::optional<std::string> unique_match(std::string cwd, std::string_view file_prefix);
