/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_file.h"
#include <assert.h>

mp4_file::mp4_file(const std::string& path)
	: mp4_container_box(MP4FILE)
	, _path(path)
{
}

mp4_file::~mp4_file()
{
}
