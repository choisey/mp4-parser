/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_playlist.h"
#include "mp4_url_encode.h"

class mp4_hls_m3u8 : public mp4_playlist, public mp4_url_encode {
	public:
		mp4_hls_m3u8(uint32_t);
		virtual ~mp4_hls_m3u8();

	public:
		virtual void execute(std::vector<std::shared_ptr<mp4_file>>&);
};
