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
                virtual ~io_local_file();

        protected:
                FILE* _fp;

        public:
                virtual std::shared_ptr<io_file::block> allocate_block(size_t = 0);

                virtual bool open(const std::string&);
                virtual bool is_open() const { return ( NULL != _fp ); }
                virtual off_t position() const;
                virtual bool seek(off_t, int);
                virtual size_t read(void*, size_t);
                virtual void close();
};
