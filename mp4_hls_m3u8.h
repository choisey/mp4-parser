/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_playlist.h"
#include "mp4_url_encode.h"

class MP4HlsM3u8 : public MP4Playlist, public MP4URLEncode {
	public:
		MP4HlsM3u8(uint32_t);
		virtual ~MP4HlsM3u8();

	public:
		virtual void execute(std::vector<std::shared_ptr<MP4File>>&);
};
