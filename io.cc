/*
 * Copyright (c) Seungyeob Choi
 */

#include "io.h"
#include "io_file.h"
#include <string.h>

// Singleton

io* io::_instance = NULL;

io* io::instance()
{
	if ( NULL == _instance ) {
		_instance = new io();
	}
	return _instance;
}

// io

io::io()
{
}

io::~io()
{
}

std::shared_ptr<io_file> io::open(const std::string& path)
{
        std::shared_ptr<io_file> f = std::make_shared<io_file>(path);
        return f;
}
