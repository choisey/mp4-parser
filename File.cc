/*
 * Copyright (c) Seungyeob Choi
 */

#include "File.h"

// File

File::File()
	: _size(0)
{
}

File::File(const std::string& uri)
	: _uri(uri)
	, _size(0)
{
}

File::~File()
{
}
