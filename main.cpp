#include "shell/shell.h"

#include <string>
#include <vector>
#include <locale>

int main(int argc, const char* argv[]) {
    setlocale(LC_ALL, "en_CA.UTF-8");
    int ret = shell(argc, argv).main_loop();
    return ret;
}
