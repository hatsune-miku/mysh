//
// Created by Zhen Guan on 2023-03-15.
//

#include "shell.h"

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <iomanip>
#include <sys/ioctl.h>

#include "util/env_util.h"
#include "util/string_util.h"
#include "util/sig_util.h"

using std::vector;
using std::string;

constexpr int STATUS_NOTHING_TO_DO = -1;
constexpr int STATUS_EXIT = 0;

auto& ttyout = std::cout;
auto& ttyerr = std::cerr;

struct termios termios_backup;
std::function<void(int sig)> signal_proxy;

static void signal_proxy_wrapper(int sig) {
    signal_proxy(sig);
}

static void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios_backup);
}

static void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &termios_backup);
    atexit(disable_raw_mode);
    struct termios raw = termios_backup;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
int get_console_width() {
    // TIOCGWINSZ: Terminal Input/Output Control Get Window Size
    // Who invented this super cool name?
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

static char read_key() {
    char c;
    read(STDIN_FILENO, &c, 1);
    return c;
}

static int execute(env* e, const vector<std::string>& args) {
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
        exit(execvp(argv[0], argv));
    }

    int status;
    e->child_process_id = pid;
    waitpid(pid, &status, WUNTRACED);
    return status;
}

/**
 * Handle a line of input
 * Returns true if the line was actually handled
 * Returns false if the line was empty
*/
static bool handle_line(env* e, const std::string& line) {
    const std::string_view trimmed_view = string_strip(line);
    const std::string trimmed(trimmed_view.begin(), trimmed_view.end());
    if (trimmed.empty()) {
        return false;
    }

    const vector<std::string> args = string_split(trimmed, " ");
    if(args.size() == 0) {
        return false;
    }

    int code = execute(e, args);

    if (WIFEXITED(code)) {
        switch (WEXITSTATUS(code)) {
        case 0:
            break;
        case 255:
            ttyerr << args[0] << ": Command not found" << "\n";
            break;
        default:
            ttyerr << "Exited with code " << WEXITSTATUS(code) << "\n";
            break;
        }
    } else if (WIFSIGNALED(code)) {
        ttyerr << "Killed by signal " << WTERMSIG(code)
            << " (" << strsignal(WTERMSIG(code)) << ")" << "\n";
    } else if (WIFSTOPPED(code)) {
        ttyerr << "Suspended by signal " << WSTOPSIG(code) 
            << " (" << strsignal(WSTOPSIG(code)) << ")" << "\n";
    } else {
        ttyerr << "Unknown exit status" << "\n";
    }
    return true;
}

#define DEFINE_KEY_HANDLER(name) \
    int handle_key_##name(shell* sh)

#define DEFINE_KEY_COMBO_HANDLER(key0, key1, key2) \
    int handle_key_##key0##_##key1##_##key2(shell* sh)

#define DEFINE_BUILTIN_COMMAND(name) \
    int builtin_command_##name(shell* sh, const vector<std::string>& args)

#define ADD_KEYMAP(key) \
    this->keymap[key] = handle_key_##key

#define ADD_KEY_COMBO_MAP(key0, key1, key2) \
    this->key_combo_map[std::make_tuple(key0, key1, key2)] = handle_key_##key0##_##key1##_##key2

#define ADD_BUILTIN_COMMAND(name) \
    this->builtin_commands[#name] = builtin_command_##name

////////// Builtin Commands //////////
DEFINE_BUILTIN_COMMAND(exit) {
    exit(0);
}

DEFINE_BUILTIN_COMMAND(cd) {
    if (args.size() == 1) {
        chdir(getenv(CONFIG_ENV_HOME));
    } else {
        chdir(args[1].c_str());
    }
    return 0;
}

DEFINE_BUILTIN_COMMAND(engi9875) {
    ttyout << "ENGI-9875 Assignment #4\nStudent: Zhen Guan (202191382)" << "\n";
    return 0;
}

DEFINE_BUILTIN_COMMAND(history) {
    for (size_t i = 0; i < sh->history.size(); i++) {
        ttyout << i << ": " << sh->history[i] << "\n";
    }
    return 0;
}

////////// Builtin Commands //////////


////////// Tab ///////////
DEFINE_KEY_HANDLER(9) {
    std::string cwd = sh->get_cwd();
    std::optional<std::string> match = sh->completion(*sh->e->input_buffer);

    if (match.has_value()) {
        sh->assign_input(match.value());
    }

    return STATUS_NOTHING_TO_DO;
}
////////// Tab ///////////


////////// Arrows //////////
DEFINE_KEY_HANDLER(KEY_UP) {
    if (sh->history_index > 0) {
        --sh->history_index;
        sh->assign_input(sh->history[sh->history_index]);
    }
    return STATUS_NOTHING_TO_DO;
}

DEFINE_KEY_HANDLER(KEY_DOWN) {
    if (sh->history_index < sh->history.size()) {
        ++sh->history_index;

        if (sh->history_index == sh->history.size()) {
            sh->clear_input();
        } else {
            sh->assign_input(sh->history[sh->history_index]);
        }
    }
    return STATUS_NOTHING_TO_DO;
}

DEFINE_KEY_HANDLER(KEY_LEFT) {
    if (sh->e->cursor > 0) {
        --sh->e->cursor;
    }
    return STATUS_NOTHING_TO_DO;
}

DEFINE_KEY_HANDLER(KEY_RIGHT) {
    if (sh->e->cursor < sh->e->input_buffer->length()) {
        ++sh->e->cursor;
    }
    else {
        handle_key_9(sh);
    }
    return STATUS_NOTHING_TO_DO;
}

// up,down,left,right: 65,66,68,67
DEFINE_KEY_COMBO_HANDLER(27, 91, 65) { return handle_key_KEY_UP(sh); }

DEFINE_KEY_COMBO_HANDLER(27, 91, 66) { return handle_key_KEY_DOWN(sh); }

DEFINE_KEY_COMBO_HANDLER(27, 91, 68) { return handle_key_KEY_LEFT(sh); }

DEFINE_KEY_COMBO_HANDLER(27, 91, 67) { return handle_key_KEY_RIGHT(sh); }
////////// Arrows //////////


////////// Del ///////////
DEFINE_KEY_COMBO_HANDLER(27, 91, 51) {
    // 27,91,51,126. Del has special handling
    if (read_key() == 126) {
        sh->e->input_buffer->erase(sh->e->cursor, 1);
    }
    return STATUS_NOTHING_TO_DO;
}
////////// Del ///////////


////////// Enter ///////////
DEFINE_KEY_HANDLER(10) {
    disable_raw_mode();

    // No matter what, we need to print a new line
    ttyout << "\n";

    const std::string_view trimmed_view = string_strip(*sh->e->input_buffer);
    const std::string trimmed(trimmed_view.begin(), trimmed_view.end());
    const auto parts = string_split(trimmed, " ");

    if (trimmed.empty() || parts.empty()) {
        enable_raw_mode();
        return STATUS_NOTHING_TO_DO;
    }

    // History
    if (sh->builtin_commands.contains(parts.front())) {
        sh->builtin_commands[parts.front()](sh, parts);
    } else if (handle_line(sh->e, *sh->e->input_buffer)) {
        sh->history.push_back(*sh->e->input_buffer);
        sh->history_index = sh->history.size();

        if (sh->history.size() >= CONFIG_HISTORY_MAX) {
            sh->history.pop_front();
        }
    }
    enable_raw_mode();

    sh->clear_input();
    return STATUS_NOTHING_TO_DO;
}

DEFINE_KEY_HANDLER(13) {
    return handle_key_10(sh);
}
////////// Enter ///////////


////////// Backspace ///////////
DEFINE_KEY_HANDLER(127) {
    if (sh->e->cursor > 0) {
        sh->e->input_buffer->erase(--sh->e->cursor, 1);
    }
    return STATUS_NOTHING_TO_DO;
}

////////// Backspace ///////////


////////// Ctrl + C ///////////
DEFINE_KEY_HANDLER(3) {
    // Ctrl + C
    ttyout << "^C\n";
    sh->clear_input();
    return STATUS_NOTHING_TO_DO;
}
////////// Ctrl + C ///////////


////////// Escape ///////////
DEFINE_KEY_HANDLER(27) {
    auto tuple = std::make_tuple(27, read_key(), read_key());
    if (sh->key_combo_map.contains(tuple)) {
        return sh->key_combo_map[tuple](sh);
    }

    return STATUS_NOTHING_TO_DO;
}
////////// Escape ///////////


shell::shell(int argc, const char* argv[]) {
    this->e = new env();

    ADD_KEYMAP(127);
    ADD_KEYMAP(27);
    ADD_KEYMAP(3);
    ADD_KEYMAP(9); // tab
    ADD_KEYMAP(10);
    ADD_KEYMAP(13);

    ADD_KEY_COMBO_MAP(27, 91, 51); // 126
    ADD_KEY_COMBO_MAP(27, 91, 65);
    ADD_KEY_COMBO_MAP(27, 91, 66);
    ADD_KEY_COMBO_MAP(27, 91, 68);
    ADD_KEY_COMBO_MAP(27, 91, 67);

    ADD_BUILTIN_COMMAND(exit);
    ADD_BUILTIN_COMMAND(cd);
    ADD_BUILTIN_COMMAND(engi9875);
    ADD_BUILTIN_COMMAND(history);
}

shell::~shell() {
    delete this->e;
}

int shell::handle_printable_key(int key) {
    this->e->input_buffer->insert(e->input_buffer->begin() + this->e->cursor++, static_cast<char>(key));
    return STATUS_NOTHING_TO_DO;
}

int shell::handle_control_key(int key) {
    if (this->keymap.contains(key)) {
        return this->keymap[key](this);
    }
    return STATUS_NOTHING_TO_DO;
}

void shell::clear_input() {
    this->e->input_buffer->clear();
    this->e->cursor = 0;
}

void shell::assign_input(const std::string& input) {
    this->e->input_buffer->assign(input);
    this->e->cursor = this->e->input_buffer->length();
}

/**
 * @param key
 * @return -1 if nothing to do, other as exit code
 */
int shell::dispatch(int key) {
    if (iscntrl(key)) {
        return handle_control_key(key);
    } else if (isprint(key)) {
        return handle_printable_key(key);
    }

    return STATUS_NOTHING_TO_DO;
}

std::string shell::get_cwd() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    return cwd;
}

std::optional<std::string> shell::completion(const std::string& input) {
    // Has unique match with in cwd or history?
    if (input.empty()) {
        return std::nullopt;
    }

    std::string prefix;
    const auto parts = string_split(input, " ");
    if (parts.size() > 0) {
        prefix = string_join(std::vector(parts.begin(), parts.end() - 1), " ") + " ";
    }

    std::optional<std::string> match = unique_match(get_cwd(), parts.back());
    if (match.has_value()) {
        return prefix + match.value();
    }

    for (const auto& h : this->history) {
        if (h.find(input) == 0) {
            return h;
        }
    }

    return std::nullopt;
}

void shell::render_input_area() {
    const auto cwd_full = get_cwd();
    const auto cwd = string_split(cwd_full, "/").back();
    const auto ps = std::string("$ ")
        .append(string_color(cwd, COLOR_BRIGHT_CYAN))
        .append(" > ");
    std::string hint;

    // Preprocess input buffer
    const auto input_buffer = *this->e->input_buffer;
    const auto input_buffer_trimmed_view = string_strip(input_buffer);
    const auto input_buffer_trimmed = std::string(input_buffer_trimmed_view.begin(), input_buffer_trimmed_view.end());

    auto space_location = input_buffer.find(" ");
    if (space_location == std::string::npos) {
        space_location = input_buffer.length();
    }

    const auto parts = string_split(input_buffer_trimmed, " ");
    hint = completion(input_buffer).value_or("");
    if (!hint.empty()) {
        hint = hint.substr(input_buffer.length());
    }

    // Found in builtin commands or PATH?
    std::string first_part = input_buffer.substr(0, space_location);
    const int color = (builtin_commands.contains(parts.front()) || is_in_PATH(parts.front()))
        ? COLOR_BRIGHT_GREEN 
        : COLOR_BRIGHT_RED;

    // Original untrimmed remaining part
    std::string remaining_part;    
    if (space_location < input_buffer.length() - 1) {
        remaining_part = input_buffer.substr(space_location);
    }

    std::string buffer = string_color(first_part, color) + remaining_part;

    ttyout << "\r" 
        << std::string(get_console_width() - ps.length(), ' ') 
        << "\r"
        << ps
        << buffer
        << string_color(hint, COLOR_GREY)
        << "\r"
        << ps
        << string_color(input_buffer.substr(0, this->e->cursor), color, first_part.length()) /* adjust cursor */
        << std::flush;
}

int shell::main_loop() {
    int key;
    int status;

    signal_proxy = [this] (int sig) {
        kill(e->child_process_id, sig);
    };

    signal(SIGINT, signal_proxy_wrapper);
    signal(SIGTSTP, signal_proxy_wrapper);

    // Warmup
    is_in_PATH("dd");

    enable_raw_mode();
    render_input_area();

    for (;;) {
        if ((key = read_key()) > 0) {
            status = dispatch(key);
            if (status != STATUS_NOTHING_TO_DO) {
                break;
            }

            render_input_area();
        }
    }

    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);

    disable_raw_mode();

    return status;
}
