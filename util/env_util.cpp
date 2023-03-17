//
// Created by Hatsune Miku on 2023-03-15.
//

#include "env_util.h"

#include <filesystem>
#include <sstream>
#include <map>
#include <vector>

#include "util/string_util.h"

namespace fs = std::filesystem;

static std::map<std::string, std::vector<std::string> > path_cache;

static inline bool is_same_file(std::string_view v0, std::string_view v1) {
    if (v0.length() != v1.length()) {
        return false;
    }

    for (size_t i = 0; i < v0.length(); i++) {
        if (std::tolower(v0[i]) != std::tolower(v1[i])) {
            return false;
        }
    }

    return true;
}

std::optional<std::string> unique_match(std::string cwd, std::string_view file_prefix) {
    std::optional<std::string> ret = std::nullopt;
    bool found = true;

    if (path_cache.contains(cwd)) {
        for (const std::string& f : path_cache[cwd]) {
            if (f.find(file_prefix) == 0) {
                if (ret.has_value()) {
                    // Not unique
                    return std::nullopt;
                }
                ret = f;
            }
        }
        return ret;
    }

    try {
        for (const auto& entry : fs::directory_iterator(cwd)) {
            auto path = entry.path().filename();
            std::string filename = path.string();
            path_cache[cwd].push_back(filename);
            if (filename.find(file_prefix) == 0) {
                if (ret.has_value()) {
                    // Not unique
                    found = false;
                }
                ret = filename;
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        // Does not exist?
    }

    return found ? ret : std::nullopt;
}

bool is_in_PATH(std::string_view file) {
    if (!path_cache.empty()) {
        for (const auto& [_, files] : path_cache) {
            for (const std::string& f : files) {
                if (is_same_file(f, file)) {
                    return true;
                }
            }
        }
        return false;
    }

    const char* path = std::getenv("PATH");
    std::vector<std::string> paths = string_split(path, ":");

    // Iter over each item in PATH
    for (const std::string& s : paths) {

        // Scan path and cache it
        try {
            for (const auto& entry : fs::directory_iterator(s)) {
                auto path = entry.path().filename();
                std::string filename = path.string();
                path_cache[s].push_back(filename);
                if (is_same_file(filename, file)) {
                    return true;
                }
            }
        }
        catch (const fs::filesystem_error& e) {
            // Does not exist?
        }
    }
    return false;
}
