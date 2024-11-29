#include "MP4File.h"
#include <assert.h>

MP4File::MP4File(const std::string& path)
	: MP4ContainerBox(MP4FILE)
	, _path(path)
{
}

MP4File::~MP4File()
{
}
