//
// Created by Hatsune Miku on 2023-03-15.
//

#include "env_util.h"

#include <filesystem>
#include <sstream>
#include <map>
#include <vector>

namespace fs = std::filesystem;

static std::map<std::string, std::vector<std::string> > path_cache;

bool is_in_PATH(std::string_view file) {
    return true;

    if (!path_cache.empty()) {
        return std::any_of(path_cache["PATH"].begin(), path_cache["PATH"].end(), [&](const std::string& s) {
            return fs::exists(fs::path(s) / file);
        });
    }

    const char* path = std::getenv("PATH");
    std::stringstream ss(path);
    path_cache["PATH"] = std::vector<std::string>();

    for (std::string s; getline(ss, /*out*/ s, ':');) {
        path_cache["PATH"].push_back(s);
        if (fs::exists(fs::path(s) / file)) {
            return true;
        }
    }
    return false;
}
