/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_playlist.h"

class mp4_hls_media_playlist : public mp4_playlist {
	public:
		mp4_hls_media_playlist(uint32_t, uint32_t);
		~mp4_hls_media_playlist() override;

	protected:
		uint32_t _track_id;

	public:
		void execute(std::shared_ptr<mp4_abstract_box>) override;
		void execute(std::vector<std::shared_ptr<mp4_file>>&) override;
};
