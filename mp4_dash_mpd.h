/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_playlist.h"
#include "mp4_codec_conf.h"
#include "mp4_url_encode.h"

class mp4_dash_mpd : public mp4_playlist, public mp4_codec_conf, public mp4_url_encode {
	public:
		mp4_dash_mpd(uint32_t);
		~mp4_dash_mpd() override;

	public:
		//void execute(std::shared_ptr<mp4_abstract_box>) override;
		void execute(std::vector<std::shared_ptr<mp4_file>>&) override;
};
