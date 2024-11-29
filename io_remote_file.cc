/*
 * Copyright (c) Seungyeob Choi
 */

#include "io_remote_file.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

//#define BUFFER_SIZE	( 1024 * 1024 * 1 ) // 1MB
#define BUFFER_SIZE	( 1024 * 512 ) // 512KB

size_t io_remote_file::curl_header_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	// This function is called by cUrl library with line-by-line data from the header.

	io_remote_file* pio_remote_file = (io_remote_file*) stream;

	// TO DO : check if pio_remote_file is still valid.

	if ( pio_remote_file )
	{
		try
		{
			size_t bytes = size * nmemb;
			if ( pio_remote_file->onHeader(ptr, bytes) )
			{
				return bytes;
			}
		}
		catch ( ... )
		{
			// do nothing...
		}
	}

	return 0;
}

size_t io_remote_file::curl_download_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	// This function gets called by libcurl as soon as there is data received that needs to be saved.
	// The size of the data pointed to by ptr is size multiplied with nmemb, it will not be zero terminated.
	// Return the number of bytes actually taken care of. If that amount differs from the amount passed to your function,
	// it'll signal an error to the library and it will abort the transfer and return CURLE_WRITE_ERROR.

	io_remote_file* pio_remote_file = (io_remote_file*) stream;

	// TO DO : check if pio_remote_file is still valid.

	if ( pio_remote_file )
	{
		try
		{
			size_t bytes = size * nmemb;
			if ( pio_remote_file->onContent(ptr, bytes) )
			{
				return bytes;
			}
		}
		catch ( ... )
		{
			// do nothing...
		}
	}

	return 0;
}

#ifdef _DEBUG
int io_remote_file::curl_debug_callback(CURL* cp, curl_infotype type, char* ptr, size_t size, void* data)
{
	switch ( type )
	{
		case CURLINFO_TEXT:
			// The data is informational text.
			break;

		case CURLINFO_HEADER_IN:
			// The data is header (or header-like) data received from the peer.
			{
				std::string str(ptr, size);
				str.erase(str.find_last_not_of("\t\r\n") + 1);
				//DEBUG("header-in:%s", str.c_str());
				fprintf(stderr, "< %s\n", str.c_str());
				break;
			}

		case CURLINFO_HEADER_OUT:
			// The data is header (or header-like) data sent to the peer.
			{
				std::string str(ptr, size);
				str.erase(str.find_last_not_of("\t\r\n") + 1);
				//DEBUG("header-out:%s", str.c_str());
				fprintf(stderr, "--\n%s\n--\n", str.c_str());
				break;
			}

		case CURLINFO_DATA_IN:
			// The data is protocol data received from the peer.
		case CURLINFO_DATA_OUT:
			//The data is protocol data sent to the peer.
			break;

		default:
			break;
	}

	return 0;
}
#endif

// io_remote_file

io_remote_file::io_remote_file()
	: _cp(NULL)
{
}

io_remote_file::io_remote_file(const std::string& uri)
	: io_file(uri)
	, _cp(NULL)
{
	open(uri);
}

io_remote_file::~io_remote_file()
{
	close();
}

std::shared_ptr<io_file::block> io_remote_file::allocate_block(size_t size)
{
	return std::make_shared<io_file::block>( ( 0 != size ) ? size : BUFFER_SIZE );
}

bool io_remote_file::onHeader(void* ptr, size_t size)
{
	std::string header_line((char*) ptr, size);

	// trim-right
	header_line.erase(header_line.find_last_not_of("\t\r\n") + 1);

	// ----
	// [1] END OF HEADER
	//		: check if the line is empty (indicating the end of header)
	// ----

	if ( header_line.empty() )
	{
		switch ( _code )
		{
			case 200: // HTTP/1.1 200 OK
			case 206: // HTTP/1.1 206 Partial Content
				return true;

			default:
				return true;
		}
	}

	// ----
	// [2] BEGINNING OF HEADER
	//		: check if the server returned an error.
	// ----

	if ( header_line.substr(0, 5) == "HTTP/" )
	{
		std::string::size_type cpos = header_line.find(" ");

		if ( std::string::npos != cpos )
		{
			std::string str = header_line.substr(cpos + 1);
			_code = atoi( str.c_str() );

			switch ( _code )
			{
				case 200: // HTTP/1.1 200 OK
				case 206: // HTTP/1.1 206 Partial Content
					return true;

				default:
					// 404 or other HTTP error
					// should not report the error immediately,
					// because we still have more data to receive!
					return true;
			}
		}

		// invalid header
		return false;
	}

	// ----
	// [3] REGULAR HEADER LINE
	//		: parse the header_line of "Name: Value" pair and store it in [m_HeaderMap]
	// ----

	std::string::size_type pos = header_line.find(": ");

	if ( std::string::npos != pos )
	{
		std::string name = header_line.substr(0, pos);

		if ( name == "Content-Length" ) {
			std::string contentLength = header_line.substr(pos + 2);
			_size = atoll( contentLength.c_str() );
		}
	}

	return true;
}

bool io_remote_file::onContent(void* ptr, size_t size)
{
	if ( NULL != ptr && 0 < size )
	{
		// if the returning data are more than requested, then trim it.
		// but, if it really happens, it must be a bug.
		assert( !(_buffer.size < _buffer.filled + size) );
		if ( _buffer.size < _buffer.filled + size )
		{
			size = _buffer.size - _buffer.filled;
		}

		memcpy((char*) _buffer.p + _buffer.filled, ptr, size);
		_buffer.filled += size;
		return true;
	}

	// TO DO: what if the connection fails in the middle of a read??

	// NULL pointer or invalid size.
	// how can it be? may need an error handling here.

	return false;
}

bool io_remote_file::open(const std::string& uri)
{
	assert( NULL == _cp );
	if ( NULL != _cp ) {
		// already open
		return false;
	}

	_cp = curl_easy_init();

//#ifdef _DEBUG
//	curl_easy_setopt(_cp, CURLOPT_VERBOSE, 1);
//	curl_easy_setopt(_cp, CURLOPT_DEBUGFUNCTION, curl_debug_callback);
//	curl_easy_setopt(_cp, CURLOPT_DEBUGDATA, NULL);
//#else
	curl_easy_setopt(_cp, CURLOPT_VERBOSE, 0);
//#endif

	curl_easy_setopt(_cp, CURLOPT_HEADER, 0);
	curl_easy_setopt(_cp, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(_cp, CURLOPT_NOSIGNAL, 1);

	curl_easy_setopt(_cp, CURLOPT_URL, uri.c_str());
	curl_easy_setopt(_cp, CURLOPT_PROTOCOLS, CURLPROTO_HTTP);
	curl_easy_setopt(_cp, CURLOPT_DNS_USE_GLOBAL_CACHE, 0);

	curl_easy_setopt(_cp, CURLOPT_AUTOREFERER, 0);
	curl_easy_setopt(_cp, CURLOPT_ENCODING, "");
	curl_easy_setopt(_cp, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(_cp, CURLOPT_MAXREDIRS, 0);
	curl_easy_setopt(_cp, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(_cp, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

	curl_easy_setopt(_cp, CURLOPT_TIMEOUT, TIMEOUT); // in seconds
	//curl_easy_setopt(_cp, CURLOPT_TIMEOUT_MS, 1); // in milliseconds
	//curl_easy_setopt(_cp, CURLOPT_LOW_SPEED_LIMIT, 100); // bytes per second
	//curl_easy_setopt(_cp, CURLOPT_LOW_SPEED_TIME, 15); // duration
	curl_easy_setopt(_cp, CURLOPT_CONNECTTIMEOUT, CONNECTION_TIMEOUT); // in seconds
	//curl_easy_setopt(_cp, CURLOPT_CONNECTTIMEOUT_MS, 1); // in milliseconds
	curl_easy_setopt(_cp, CURLOPT_USERAGENT, USERAGENT);

	// additional header lines in {slist}
	//char buf[50];
	//snprintf(buf, sizeof(buf), "Request-Id: %lld", m_id);
	//struct curl_slist *slist = NULL;
	//slist = curl_slist_append(slist, buf);
	//curl_easy_setopt(_cp, CURLOPT_HTTPHEADER, slist);

	curl_easy_setopt(_cp, CURLOPT_HEADERFUNCTION, curl_header_callback);
	curl_easy_setopt(_cp, CURLOPT_HEADERDATA, (void*) this);
	curl_easy_setopt(_cp, CURLOPT_WRITEFUNCTION, NULL);
	curl_easy_setopt(_cp, CURLOPT_WRITEDATA, NULL);

	curl_easy_setopt(_cp, CURLOPT_NOBODY, 1);
	curl_easy_setopt(_cp, CURLOPT_RANGE, NULL);

	_size = 0xffffffff;
	_code = 0;

	curl_easy_perform(_cp);

	switch ( _code ) {
		case 200:
			_position = 0;
			return true;

		default:
			close();
			return false;
	}
}

bool io_remote_file::seek(off_t offset, int origin)
{
	switch ( origin ) {
		case SEEK_SET:
			assert( 0 <= offset );
			_position = offset;
			return true;

		case SEEK_CUR:
			_position += offset;
			return true;

		case SEEK_END:
			assert( offset <= 0 );
			_position = _size + offset;
			return true;

		default:
			assert(false);
	}

	return false;
}

size_t io_remote_file::read(void* buf, size_t size)
{
	assert( NULL != _cp );

	if ( NULL == _cp ) {
		return 0;
	}

	assert((size_t) _position < _size );
	assert( 1 <= size );

	if ( _size <= (size_t) _position ) {
		return 0;
	}

	curl_easy_setopt(_cp, CURLOPT_HEADERFUNCTION, NULL);
	curl_easy_setopt(_cp, CURLOPT_HEADERDATA, NULL);
	curl_easy_setopt(_cp, CURLOPT_NOBODY, 0);

	// This size is by default set as big as possible (CURL_MAX_WRITE_SIZE),
	// so it only makes sense to use this option if you want it smaller.
	//curl_easy_setopt(_cp, CURLOPT_BUFFERSIZE, CURL_MAX_WRITE_SIZE);

	char range[40];
	sprintf(range, "%ld-%ld", _position, _position + size - 1);

	_buffer.p = buf;
	_buffer.size = size;
	_buffer.filled = 0;

	curl_easy_setopt(_cp, CURLOPT_WRITEFUNCTION, curl_download_callback);
	curl_easy_setopt(_cp, CURLOPT_WRITEDATA, (void*) this);
	curl_easy_setopt(_cp, CURLOPT_RANGE, range);
	curl_easy_perform(_cp);

	_position += _buffer.filled;
	return _buffer.filled;
}

void io_remote_file::close()
{
	if ( _cp )
	{
		curl_easy_cleanup( _cp );
		_cp = NULL;
	}
}
