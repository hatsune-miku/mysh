//
// Created by Hatsune Miku on 2023-03-15.
//

#include "shell.h"

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include <ncurses.h>
#include "util/env_util.h"

using std::vector;
using std::string;

constexpr int STATUS_NOTHING_TO_DO = -1;
constexpr int STATUS_EXIT = 0;

int execute(const vector<std::string> &args) {

    char* argv[args.size() + 1];
    for (size_t i = 0; i < args.size(); i++) {
        argv[i] = (char*) args[i].c_str();
    }
    argv[args.size()] = nullptr;

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }
}

void handle_line(const std::string &line) {
    vector<std::string> args; // = line.split(" ");
    if (args[0] == CONFIG_COMMAND_EXIT) {
    } else if (args[0] == CONFIG_COMMAND_CHDIR) {
        if (args.size() == 1) {
            chdir(getenv(CONFIG_ENV_HOME));
        } else {
            chdir(args[1].c_str());
        }
    } else {
        int status = execute(args);
        if (status == -1) {
            std::cerr << "Error executing command" << std::endl;
        } else {
            std::cout << "Command returned status " << status << std::endl;
        }
    }
}

#define DEFINE_KEY_HANDLER(name) \
    void handle_key_##name(shell* sh)

#define DEFINE_KEY_COMBO_HANDLER(key0, key1, key2) \
    void handle_key_##key0##_##key1##_##key2(shell* sh)

#define ADD_KEYMAP(key) \
    this->keymap[key] = handle_key_##key

#define ADD_KEY_COMBO_MAP(key0, key1, key2) \
    this->key_combo_map[std::make_tuple(key0, key1, key2)] = handle_key_##key0##_##key1##_##key2

////////// Arrows //////////
DEFINE_KEY_HANDLER(KEY_UP) {

}

DEFINE_KEY_HANDLER(KEY_DOWN) {

}

DEFINE_KEY_HANDLER(KEY_LEFT) {
    if (sh->e->cursor > 0) {
        --sh->e->cursor;
    }
}

DEFINE_KEY_HANDLER(KEY_RIGHT) {
    if (sh->e->cursor < sh->e->input_buffer->length()) {
        ++sh->e->cursor;
    }
}

// up,down,left,right: 65,66,68,67
DEFINE_KEY_COMBO_HANDLER(27, 91, 65) { handle_key_KEY_UP(sh); }

DEFINE_KEY_COMBO_HANDLER(27, 91, 66) { handle_key_KEY_DOWN(sh); }

DEFINE_KEY_COMBO_HANDLER(27, 91, 68) { handle_key_KEY_LEFT(sh); }

DEFINE_KEY_COMBO_HANDLER(27, 91, 67) { handle_key_KEY_RIGHT(sh); }
////////// Arrows //////////


////////// Enter ///////////
DEFINE_KEY_HANDLER(KEY_ENTER) {

}

DEFINE_KEY_HANDLER(10) {
    handle_key_KEY_ENTER(sh);
}
////////// Enter ///////////


////////// Backspace ///////////
DEFINE_KEY_HANDLER(KEY_BACKSPACE) {
    if (sh->e->cursor > 0) {
        sh->e->input_buffer->erase(--sh->e->cursor, 1);
    }
}

DEFINE_KEY_HANDLER(127) {
    handle_key_KEY_BACKSPACE(sh);
}
////////// Backspace ///////////


shell::shell(int argc, const char* argv[]) {
    this->e = new env();
    ADD_KEYMAP(KEY_LEFT);
    ADD_KEYMAP(KEY_RIGHT);
    ADD_KEYMAP(KEY_BACKSPACE);
    ADD_KEYMAP(127);
    ADD_KEY_COMBO_MAP(27, 91, 65);
    ADD_KEY_COMBO_MAP(27, 91, 66);
    ADD_KEY_COMBO_MAP(27, 91, 68);
    ADD_KEY_COMBO_MAP(27, 91, 67);

    win_width = getmaxx(stdscr);
    win_height = getmaxy(stdscr);

    // TODO: review line height limit?
    win_input = newwin(2, win_width, 0, 0);
    win_output = newwin(win_height - 1, win_width, 1, 0);

    scrollok(win_output, true);
}

void shell::on_resized(int new_input_point_row, int new_input_point_col) {
    // Horizontal: X
    // Vertical: Y
    win_width = getmaxx(stdscr);
    win_height = getmaxy(stdscr);

    wrefresh(win_input);
    wrefresh(win_output);

    wmove(win_input, new_input_point_row, new_input_point_col);
    wmove(win_output, new_input_point_row + 1, 0);

    scrollok(win_output, true);
}

shell::~shell() {
    delete this->e;
}

int shell::handle_normal_key(int key) {
    this->e->input_buffer->insert(e->input_buffer->begin() + this->e->cursor, static_cast<char>(key));
    ++this->e->cursor;
    return STATUS_NOTHING_TO_DO;
}

int shell::handle_control_key(int key) {
    if (key == 3) {
        // Ctrl + C
        return STATUS_EXIT;
    }
    if (key == 27) {
        // Escape
        auto tuple = std::make_tuple(key, wgetch(win_input), wgetch(win_input));
        if (this->key_combo_map.contains(tuple)) {
            this->key_combo_map[tuple](this);
            return STATUS_NOTHING_TO_DO;
        }
    }
    if (key == KEY_ENTER) {
        // render_input_area();
    }

    std::cerr << "key: " << key << ", cursor: " << e->cursor << std::endl;

    if (this->keymap.contains(key)) {
        this->keymap[key](this);
    }
    return STATUS_NOTHING_TO_DO;
}

/**
 * @param key
 * @return -1 if nothing to do, other as exit code
 */
int shell::dispatch(int key) {
    if (iscntrl(key)) {
        return handle_control_key(key);
    }

    if (isprint(key)) {
        return handle_normal_key(key);
    }

    return STATUS_NOTHING_TO_DO;
}

std::string shell::get_cwd() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    return cwd;
}

void shell::render_input_area() {
    char ps[1024];
    std::string cwd_path = get_cwd();
    std::string_view cwd(cwd_path.begin() + cwd_path.rfind('/'), cwd_path.end());

    int len = snprintf(ps, sizeof(ps), "➜ %s ", cwd.data());

    wclear(win_input);

    wattron(win_input, COLOR_PAIR(COLOR_SECONDARY));
    mvwprintw(win_input, 0, 0, "%s", ps);
    wattroff(win_input, COLOR_PAIR(COLOR_SECONDARY));

    const std::string* input = this->e->input_buffer;
    std::string first_param(input->begin(), input->begin() + input->find(' '));
    std::string remain_param(input->begin() + input->find(' '), input->end());

    if (is_in_PATH(first_param)) {
        wattron(win_input, COLOR_PAIR(COLOR_SAFE));
        mvwprintw(win_input, 0, len, "%s", first_param.c_str());
        wattroff(win_input, COLOR_PAIR(COLOR_SAFE));
    }
    else {
        wattron(win_input, COLOR_PAIR(COLOR_DANGER));
        mvwprintw(win_input, 0, len, "%s", first_param.c_str());
        wattroff(win_input, COLOR_PAIR(COLOR_DANGER));
    }

    mvwprintw(win_input, 0, len, "%s", remain_param.c_str());
    wmove(win_input, 0, len + this->e->cursor);
    wrefresh(win_input);
}

// 639151484352536025
int shell::main_loop() {
    int key;
    int status;

    render_input_area();

    for (;;) {
        if ((key = wgetch(win_input)) > 0) {
            status = dispatch(key);
            if (status != STATUS_NOTHING_TO_DO) {
                break;
            }
            render_input_area();
        }
    }
    return status;
}
