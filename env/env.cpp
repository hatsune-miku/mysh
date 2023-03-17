//
// Created by Hatsune Miku on 2023-03-15.
//

#include "env.h"

env::env() {
    buffer = new std::string();
    input_buffer = new std::string();
    cursor = 0;
}

env::~env() {
    delete buffer;
    delete input_buffer;
}
