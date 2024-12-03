/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include <assert.h>
#include <sys/types.h>
#include <string>
#include <memory>

class io_file
{
        public:
                class block {
                        public:
                                block(size_t size) : _size(size)
                                {
                                        assert( 0 != size );
                                        _p = new uint8_t[size];
                                }
                                ~block()
                                {
                                        assert( NULL != _p );
                                        if ( NULL != _p ) {
                                                delete _p;
                                        }
                                }

                        private:
                                uint8_t* _p;
                                size_t _size;

                        public:
                                operator uint8_t* () const { return _p; }
                                size_t size() const { return _size; }
                };

        public:
                io_file();
                io_file(const std::string&);
                virtual ~io_file();

        protected:
                FILE* _fp;

        protected:
                std::string _uri;
                size_t _size;

        public:
                std::shared_ptr<block> allocate_block(size_t = 0);

                const std::string& path() const { return _uri; }
                size_t size() const { return _size; }

                bool open(const std::string&);
                bool is_open() const { return ( NULL != _fp ); }
                off_t position() const;
                bool seek(off_t, int);
                size_t read(void*, size_t);
                void close();
};
