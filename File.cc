/*
 * Copyright (c) Seungyeob Choi
 */

#include "File.h"

// File

DCP::File::File()
	: _size(0)
{
}

DCP::File::File(const std::string& uri)
	: _uri(uri)
	, _size(0)
{
}

DCP::File::~File()
{
}
