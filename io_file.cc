/*
 * Copyright (c) Seungyeob Choi
 */

#include "io_file.h"

io_file::io_file()
	: _size(0)
{
}

io_file::io_file(const std::string& uri)
	: _uri(uri)
	, _size(0)
{
}

io_file::~io_file()
{
}
