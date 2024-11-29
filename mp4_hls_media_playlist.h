/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_playlist.h"

class mp4_hls_media_playlist : public mp4_playlist {
	public:
		mp4_hls_media_playlist(uint32_t, uint32_t);
		virtual ~mp4_hls_media_playlist();

	protected:
		uint32_t _track_id;

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
		virtual void execute(std::vector<std::shared_ptr<mp4_file>>&);
};
