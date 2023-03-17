//
// Created by Hatsune Miku on 2023-03-15.
//

#pragma once


constexpr int CONFIG_BUFFER_SIZE = 8192;
constexpr int CONFIG_INPUT_SIZE = 1024;

#define CONFIG_COMMAND_EXIT "exit"
#define CONFIG_COMMAND_CHDIR "cd"

#define CONFIG_COMMAND_LS "ls"

#define CONFIG_ENV_HOME "HOME"
#define CONFIG_ENV_PATH "PATH"

#define COLOR_PRIMARY 1
#define COLOR_SECONDARY 2
#define COLOR_DANGER 3
#define COLOR_SAFE 4
