cmake_minimum_required(VERSION 3.24)

set(
    SHSH_MOTD_SRC

    main.cpp
)

include_directories(
    motd
)

add_executable(motd ${SHSH_MOTD_SRC})
