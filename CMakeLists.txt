cmake_minimum_required(VERSION 3.24)
project(shsh)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 23)

set(
    SHSH_SRC

    main.cpp
    config.h env/env.cpp env/env.h shell/shell.cpp shell/shell.h util/env_util.cpp util/env_util.h)

include_directories(
    .
)

link_libraries(ncurses)

add_executable(shsh ${SHSH_SRC})
