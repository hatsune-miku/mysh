//
// Created by Hatsune Miku on 2023-03-15.
//

#pragma once

#include "env/env.h"

#include <ncurses.h>
#include <string>
#include <map>

class shell {
public:
    env* e;
    std::map<int, std::function<void(shell* sh)>> keymap;
    std::map<std::tuple<int, int, int>, std::function<void(shell* sh)>> key_combo_map;
    WINDOW* win_input;
    WINDOW* win_output;
    int win_width;
    int win_height;

    shell(int argc, const char* argv[]);
    ~shell();

    void render_input_area();
    void flush();
    int dispatch(int key);
    void on_resized(int new_input_point_row, int new_input_point_col);

    int handle_normal_key(int key);
    int handle_control_key(int key);

    std::string get_cwd();

    int main_loop();
};

