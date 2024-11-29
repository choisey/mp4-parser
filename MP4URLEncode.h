/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class MP4URLEncode {
	public:
		MP4URLEncode();
		virtual ~MP4URLEncode();

	protected:
		std::string url_encode(const std::string&);
};
