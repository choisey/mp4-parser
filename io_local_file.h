/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "io_file.h"
#include <stdio.h>

class io_local_file : public io_file
{
        public:
                io_local_file();
                io_local_file(const std::string&);
                ~io_local_file() override;

        protected:
                FILE* _fp;

        public:
                std::shared_ptr<io_file::block> allocate_block(size_t = 0) override;

                bool open(const std::string&) override;
                bool is_open() const override { return ( NULL != _fp ); }
                off_t position() const override;
                bool seek(off_t, int) override;
                size_t read(void*, size_t) override;
                void close() override;
};
