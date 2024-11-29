/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "MP4AbstractAction.h"

class MP4URLEncode {
	public:
		MP4URLEncode();
		virtual ~MP4URLEncode();

	protected:
		std::string url_encode(const std::string&);
};
