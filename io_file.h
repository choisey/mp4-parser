/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include <assert.h>
#include <sys/types.h>
#include <string>

class io_file
{
        public:
                io_file();
                io_file(const std::string&);
                virtual ~io_file();

        protected:
                FILE* _fp{nullptr};

        protected:
                std::string _path;
                size_t _size{0};

        public:
                const std::string& path() const { return _path; }
                size_t size() const { return _size; }

                bool open(const std::string&);
                bool is_open() const { return ( NULL != _fp ); }
                off_t position() const;
                bool seek(off_t, int);
                size_t read(void*, size_t);
                void close();
};
