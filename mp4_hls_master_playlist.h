/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"
#include "mp4_codec_conf.h"
#include "mp4_url_encode.h"

class MP4HlsMasterPlaylist : public mp4_abstract_action, public MP4CodecConf, public MP4URLEncode {
	public:
		MP4HlsMasterPlaylist(uint32_t = 7);
		virtual ~MP4HlsMasterPlaylist();

	private:
		uint32_t _version;

	private:
		void langCodeToName(uint8_t, uint8_t, uint8_t, std::string&, std::string&);
		void langCodeToName(uint32_t, std::string&, std::string&);
		void hls3_master_m3u8(std::vector<std::shared_ptr<MP4File>>&);
		void hls7_master_m3u8(std::vector<std::shared_ptr<MP4File>>&);

	public:
		virtual void execute(std::vector<std::shared_ptr<MP4File>>&);
};
