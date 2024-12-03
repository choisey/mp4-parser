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
	open(uri);
}

io_file::~io_file()
{
}

std::shared_ptr<io_file::block> io_file::allocate_block(size_t size)
{
	return std::make_shared<io_file::block>( ( 0 != size ) ? size : BUFSIZ );
}

bool io_file::open(const std::string& uri)
{
	assert( NULL == _fp );
	if ( NULL != _fp ) {
		// already open
		return false;
	}

	_fp = fopen(uri.c_str(), "r");
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
