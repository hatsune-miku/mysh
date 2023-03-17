#include <signal.h>

#include "sig_util.h"

const char* strsignal(int sig) {
    switch (sig) {
    case SIGKILL:
        return "SIGKILL";
    case SIGSTOP:
        return "SIGSTOP";
    case SIGTERM:
        return "SIGTERM";
    case SIGTRAP:
        return "SIGTRAP";
    case SIGABRT:
        return "SIGABRT";
    case SIGALRM:
        return "SIGALARM";
    case SIGSEGV:
        return "SIGSEGV";
    case SIGQUIT:
        return "SIGQUIT";
    case SIGINT:
        return "SIGINT";
    case SIGCHLD:
        return "SIGCHLD";
    case SIGCONT:
        return "SIGCONT";
    case SIGPIPE:
        return "SIGPIPE";
    case SIGFPE:
        return "SIGFPE";
    case SIGILL:
        return "SIGILL";
    case SIGTSTP:
        return "SIGTSTP";
    case 0:
        return "[NO SIGNAL TO DELIVER]";
    default:
        return "UNKN";
    }
}
