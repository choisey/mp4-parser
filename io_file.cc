/*
 * Copyright (c) Seungyeob Choi
 */

#include "io_file.h"

io_file::io_file()
        : _path()
{
}

io_file::io_file(const std::string& path)
	: _path(path)
{
	open(path);
}

io_file::~io_file()
{
}

bool io_file::open(const std::string& path)
{
	assert( NULL == _fp );
	if ( NULL != _fp ) {
		// already open
		return false;
	}

	_fp = fopen(path.c_str(), "r");
	if ( NULL == _fp ) {
		return false;
	}

	fseek(_fp, 0L, SEEK_END);
	_size = ftell(_fp);
	fseek(_fp, 0L, SEEK_SET);

	return true;
}

off_t io_file::position() const
{
	assert( NULL != _fp );

	return ( NULL != _fp )
		? ftell(_fp)
		: 0;
}

bool io_file::seek(off_t offset, int origin)
{
	assert( ( SEEK_SET != origin ) || ( 0 <= offset && (size_t) offset <= _size ) );
	assert( ( SEEK_END != origin ) || ( 0 <= -offset && (size_t) -offset <= _size ) );

	return ( NULL != _fp )
		? ( 0 == fseek( _fp, offset, origin ) )
		: false;
}

size_t io_file::read(void* buf, size_t size)
{
	assert( 1 <= size );
	// It is not unusual to attempt to read beyond the upper boundary of file.
	//assert( ftell(_fp) + size <= _size );

	return ( NULL != _fp )
		? fread(buf, 1, size, _fp)
		: 0L;
}

void io_file::close()
{
	if ( NULL != _fp ) {
		fclose( _fp );
		_fp = NULL;
	}
}
