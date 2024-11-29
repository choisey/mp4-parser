/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class MP4CodecConf {
	public:
		MP4CodecConf();
		virtual ~MP4CodecConf();

	protected:
		bool decoder_conf_string(std::shared_ptr<mp4_abstract_box>, std::string&);
};
