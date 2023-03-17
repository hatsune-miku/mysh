//
// Created by Zhen Guan on 2023-03-15.
//

#include "env.h"

env::env() {
    output_buffer = new std::string();
    input_buffer = new std::string();
    child_process_id = -1;
    cursor = 0;
}

env::~env() {
    delete output_buffer;
    delete input_buffer;
}
