/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class mp4_url_encode {
	public:
		mp4_url_encode();
		virtual ~mp4_url_encode();

	protected:
		std::string url_encode(const std::string&);
};
