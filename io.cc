/*
 * Copyright (c) Seungyeob Choi
 */

#include "io.h"
#include "io_local_file.h"
#include "io_remote_file.h"
#include <curl/curl.h>
#include <string.h>

#define SCHEME_HTTP "http://"

// Singleton

io* io::_instance = NULL;

io* io::instance()
{
	if ( NULL == _instance ) {
		_instance = new io();
	}
	return _instance;
}

void io::cleanup()
{
	if ( NULL != _instance ) {
		delete _instance;
		_instance = NULL;
	}
}

// io

io::io()
{
	// If we don't properly initialize using curl_global_init, we will get seg fault.
	// https://curl.haxx.se/mail/lib-2009-11/0258.html
	curl_global_init(CURL_GLOBAL_NOTHING);
}

io::~io()
{
	curl_global_cleanup();
}

std::shared_ptr<io_file> io::open(const std::string& uri)
{
	if ( 0 == uri.compare(0, strlen(SCHEME_HTTP), SCHEME_HTTP) ) {
		std::shared_ptr<io_file> f = std::make_shared<io_remote_file>(uri);
		return f;
	}
	else {
		std::shared_ptr<io_file> f = std::make_shared<io_local_file>(uri);
		return f;
	}
}
