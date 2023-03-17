//
// Created by Hatsune Miku on 2023-03-15.
//

#pragma once

#include "config.h"

#include <string>

class env {
public:
    std::string* output_buffer;
    std::string* input_buffer;
    int child_process_id;

    int cursor;

    env();
    ~env();
};
