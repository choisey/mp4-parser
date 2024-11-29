/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_playlist.h"
#include "mp4_codec_conf.h"
#include "mp4_url_encode.h"

class MP4DashMpd : public MP4Playlist, public MP4CodecConf, public MP4URLEncode {
	public:
		MP4DashMpd(uint32_t);
		virtual ~MP4DashMpd();

	public:
		//virtual void execute(std::shared_ptr<mp4_abstract_box>);
		virtual void execute(std::vector<std::shared_ptr<MP4File>>&);
};
