cmake_minimum_required(VERSION 3.24)
project(shsh)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 23)

set(
    SHSH_SRC

    main.cpp
    env/env.cpp 
    shell/shell.cpp 
    util/env_util.cpp
    util/string_util.cpp
    util/sig_util.cpp
)

include_directories(
    .
)

add_subdirectory(motd)

add_executable(shsh ${SHSH_SRC})
