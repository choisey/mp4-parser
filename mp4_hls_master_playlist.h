/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"
#include "mp4_codec_conf.h"
#include "mp4_url_encode.h"

class mp4_hls_master_playlist : public mp4_abstract_action, public mp4_codec_conf, public mp4_url_encode {
	public:
		mp4_hls_master_playlist(uint32_t = 7);
		virtual ~mp4_hls_master_playlist();

	private:
		uint32_t _version;

	private:
		void lang_code_to_name(uint8_t, uint8_t, uint8_t, std::string&, std::string&);
		void lang_code_to_name(uint32_t, std::string&, std::string&);
		void hls3_master_m3u8(std::vector<std::shared_ptr<mp4_file>>&);
		void hls7_master_m3u8(std::vector<std::shared_ptr<mp4_file>>&);

	public:
		virtual void execute(std::vector<std::shared_ptr<mp4_file>>&);
};
