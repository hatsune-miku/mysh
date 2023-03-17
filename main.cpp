#include "shell/shell.h"
#include <ncurses.h>

#include <string>
#include <vector>
#include <locale>

int main2() {
    // Initialize curses
    initscr();
    cbreak();
    noecho();

    // Create input_buffer and output windows
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    WINDOW* input_win = newwin(1, max_x, max_y - 1, 0);
    WINDOW* output_win = newwin(max_y - 1, max_x, 0, 0);
    scrollok(output_win, true);

    // Loop until user quits
    std::vector<std::string> history;
    std::string input_buffer;
    int input_cursor = 0;
    int history_index = -1;
    while (true) {
        // Clear input_buffer window and draw prompt and input_buffer buffer
        wclear(input_win);
        mvwprintw(input_win, 0, 0, "$ ");
        mvwprintw(input_win, 0, 2, input_buffer.c_str());
        wmove(input_win, 0, 2 + input_cursor);
        wrefresh(input_win);

        // Wait for user input_buffer
        int ch = wgetch(input_win);
        if (ch == KEY_ENTER || ch == '\n') {
            // Execute command and clear input_buffer buffer
            history.push_back(input_buffer);
            system(input_buffer.c_str());
            input_buffer.clear();
            input_cursor = 0;
            history_index = -1;
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            // Delete character before cursor
            if (input_cursor > 0) {
                input_buffer.erase(input_cursor - 1, 1);
                input_cursor--;
            }
        } else if (ch == KEY_DC) {
            // Delete character at cursor
            if (input_cursor < input_buffer.length()) {
                input_buffer.erase(input_cursor, 1);
            }
        } else if (ch == KEY_LEFT) {
            // Move cursor left
            if (input_cursor > 0) {
                input_cursor--;
            }
        } else if (ch == KEY_RIGHT) {
            // Move cursor right
            if (input_cursor < input_buffer.length()) {
                input_cursor++;
            }
        } else if (ch == KEY_UP) {
            // Scroll through command history
            if (history_index < (int) history.size() - 1) {
                history_index++;
                input_buffer = history[history.size() - 1 - history_index];
                input_cursor = input_buffer.length();
            }
        } else if (ch == KEY_DOWN) {
            // Scroll through command history
            if (history_index >= 0) {
                history_index--;
                if (history_index < 0) {
                    input_buffer.clear();
                    input_cursor = 0;
                } else {
                    input_buffer = history[history.size() - 1 - history_index];
                    input_cursor = input_buffer.length();
                }
            }
        } else if (ch == KEY_RESIZE) {
            // Resize window
            endwin();
            refresh();
            getmaxyx(stdscr, max_y, max_x);
            wresize(input_win, 1, max_x);
            wresize(output_win, max_y - 1, max_x);
            mvwin(input_win, max_y - 1, 0);
        } else if (isprint(ch)) {
            // Insert printable character at cursor
            input_buffer.insert(input_cursor, 1, ch);
            input_cursor++;
        }

        // Print command output to output window
        wclear(output_win);
        for (size_t i = 0; i < history.size(); i++) {
            mvwprintw(output_win, i, 0, history[history.size() - 1 - i].c_str());
        }
        wrefresh(output_win);
    }

// Cleanup
    delwin(input_win);
    delwin(output_win);
    endwin();

    return 0;
}

int main(int argc, const char* argv[]) {
    setlocale(LC_ALL, "");

    // Initialize curses
    initscr();
    cbreak();
    noecho();

    start_color();
    init_pair(COLOR_PRIMARY, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_SECONDARY, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_SAFE, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_DANGER, COLOR_RED, COLOR_BLACK);

    int ret = shell(argc, argv).main_loop();
    return ret;
}
