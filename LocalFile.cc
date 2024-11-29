#include "LocalFile.h"
#include <assert.h>

// LocalFile

DCP::LocalFile::LocalFile()
	: _fp(NULL)
{
}

DCP::LocalFile::LocalFile(const std::string& uri)
	: File(uri)
	, _fp(NULL)
{
	open(uri);
}

DCP::LocalFile::~LocalFile()
{
	close();
}

std::shared_ptr<DCP::File::Block> DCP::LocalFile::allocateBlock(size_t size)
{
	return std::make_shared<DCP::File::Block>( ( 0 != size ) ? size : BUFSIZ );
}

bool DCP::LocalFile::open(const std::string& uri)
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

off_t DCP::LocalFile::position() const
{
	assert( NULL != _fp );

	return ( NULL != _fp )
		? ftell(_fp)
		: 0;
}

bool DCP::LocalFile::seek(off_t offset, int origin)
{
	assert( ( SEEK_SET != origin ) || ( 0 <= offset && (size_t) offset <= _size ) );
	assert( ( SEEK_END != origin ) || ( 0 <= -offset && (size_t) -offset <= _size ) );

	return ( NULL != _fp )
		? ( 0 == fseek( _fp, offset, origin ) )
		: false;
}

size_t DCP::LocalFile::read(void* buf, size_t size)
{
	assert( 1 <= size );
	// It is not unusual to attempt to read beyond the upper boundary of file.
	//assert( ftell(_fp) + size <= _size );

	return ( NULL != _fp )
		? fread(buf, 1, size, _fp)
		: 0L;
}

void DCP::LocalFile::close()
{
	if ( NULL != _fp ) {
		fclose( _fp );
		_fp = NULL;
	}
}