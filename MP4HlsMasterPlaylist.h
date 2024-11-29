/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "MP4AbstractAction.h"
#include "MP4CodecConf.h"
#include "MP4URLEncode.h"

class MP4HlsMasterPlaylist : public MP4AbstractAction, public MP4CodecConf, public MP4URLEncode {
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
