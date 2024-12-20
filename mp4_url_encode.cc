/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_url_encode.h"
#include "mp4.h"
#include <assert.h>

// mp4_url_encode

mp4_url_encode::mp4_url_encode()
{
}

mp4_url_encode::~mp4_url_encode()
{
}

std::string mp4_url_encode::url_encode(const std::string& url)
{
	std::string encoded_url = "";

	for ( const unsigned char& c : url ) {
		if ( isalnum(c)
				|| '-' == c
				|| '_' == c
				|| '.' == c
				|| '~' == c
				|| '?' == c
				|| '&' == c ) {
			encoded_url += c;
		}
		else {
			char buf[10];
			sprintf(buf, "%02X", c);
			encoded_url += '%';
			encoded_url += buf;
		}
	}

	return encoded_url;
}
