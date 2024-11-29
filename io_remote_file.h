/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "io_file.h"
#include <curl/curl.h>
#include <map>

class io_remote_file : public io_file
{
        public:
                static size_t curl_header_callback(void*, size_t, size_t, void*);
                static size_t curl_download_callback(void*, size_t, size_t, void*);
#ifdef _DEBUG
                static int curl_debug_callback(CURL*, curl_infotype, char*, size_t, void*);
#endif

        public:
                io_remote_file();
                io_remote_file(const std::string&);
                virtual ~io_remote_file();

        protected:
                const long TIMEOUT = 30;
                const long CONNECTION_TIMEOUT = 5;
                const char* USERAGENT = "ec/fms-http-3.2";

        protected:
                CURL* _cp;

                // open()
                int _code;

                // seek() and read()
                off_t _position;

                // response body
                struct {
                        void* p;
                        size_t size;
                        size_t filled;
                } _buffer;

        protected:
                bool on_header(void*, size_t);
                bool on_content(void*, size_t);

        public:
                virtual std::shared_ptr<io_file::block> allocate_block(size_t = 0);

                virtual bool open(const std::string&);
                virtual bool is_open() const { return ( NULL != _cp ); }
                virtual off_t position() const { return _position; }
                virtual bool seek(off_t, int);
                virtual size_t read(void*, size_t);
                virtual void close();
};
