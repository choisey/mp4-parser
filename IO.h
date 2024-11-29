/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "File.h"
#include <memory>

class IO {
        public:
                static IO* Instance();
                static void cleanup();

        private:
                static IO* _instance;
                IO();

        public:
                ~IO();

        public:
                std::shared_ptr<File> open(const std::string&);
};
