/*
 * Copyright (c) Seungyeob Choi
 */

#include "MP4File.h"
#include <assert.h>

MP4File::MP4File(const std::string& path)
	: mp4_container_box(MP4FILE)
	, _path(path)
{
}

MP4File::~MP4File()
{
}
