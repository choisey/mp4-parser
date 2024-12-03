/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "io_file.h"
#include <memory>

class io {
        public:
                static io* instance();

        private:
                static io* _instance;
                io();

        public:
                ~io();

        public:
                std::shared_ptr<io_file> open(const std::string&);
};
