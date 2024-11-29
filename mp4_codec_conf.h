/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class mp4_codec_conf {
	public:
		mp4_codec_conf();
		virtual ~mp4_codec_conf();

	protected:
		bool decoder_conf_string(std::shared_ptr<mp4_abstract_box>, std::string&);
};
