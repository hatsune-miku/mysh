//
// Created by Hatsune Miku on 2023-03-15.
//

#pragma once

#include "env/env.h"

#include <string>
#include <map>
#include <deque>
#include <vector>
#include <functional>
#include <optional>

class shell {
public:
    env* e;
    std::map<int, std::function<int(shell* sh)>> keymap;
    std::map<std::tuple<int, int, int>, std::function<int(shell* sh)>> key_combo_map;
    std::map<std::string, std::function<int(shell* sh, const std::vector<std::string>& args)>> builtin_commands;

    std::deque<std::string> history;
    int history_index;

    shell(int argc, const char* argv[]);
    ~shell();

    void clear_input();
    void assign_input(const std::string& input);

    std::optional<std::string> completion(const std::string& input);

    int dispatch(int key);
    void render_input_area();

    int handle_printable_key(int key);
    int handle_control_key(int key);

    std::string get_cwd();

    int main_loop();
};
