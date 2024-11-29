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
                std::string _uri;
                size_t _size;

        public:
                virtual std::shared_ptr<block> allocate_block(size_t = 0) = 0;

                const std::string& uri() const { return _uri; }
                size_t size() const { return _size; }

                virtual bool open(const std::string&) = 0;
                virtual bool is_open() const = 0;
                virtual off_t position() const = 0;
                virtual bool seek(off_t, int) = 0;
                virtual size_t read(void*, size_t) = 0;
                virtual void close() = 0;
};
