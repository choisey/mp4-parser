/*
 * Copyright (c) Seungyeob Choi
 */

#include "IO.h"
#include "LocalFile.h"
#include "RemoteFile.h"
#include <curl/curl.h>
#include <string.h>

#define SCHEME_HTTP "http://"

// Singleton

DCP::IO* DCP::IO::_instance = NULL;

DCP::IO* DCP::IO::Instance()
{
	if ( NULL == _instance ) {
		_instance = new IO();
	}
	return _instance;
}

void DCP::IO::cleanup()
{
	if ( NULL != _instance ) {
		delete _instance;
		_instance = NULL;
	}
}

// IO

DCP::IO::IO()
{
	// If we don't properly initialize using curl_global_init, we will get seg fault.
	// https://curl.haxx.se/mail/lib-2009-11/0258.html
	curl_global_init(CURL_GLOBAL_NOTHING);
}

DCP::IO::~IO()
{
	curl_global_cleanup();
}

std::shared_ptr<DCP::File> DCP::IO::open(const std::string& uri)
{
	if ( 0 == uri.compare(0, strlen(SCHEME_HTTP), SCHEME_HTTP) ) {
		std::shared_ptr<DCP::File> f = std::make_shared<DCP::RemoteFile>(uri);
		return f;
	}
	else {
		std::shared_ptr<DCP::File> f = std::make_shared<DCP::LocalFile>(uri);
		return f;
	}
}
