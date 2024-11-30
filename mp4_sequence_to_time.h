/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_multi_track_segment.h"
#include "mp4_playlist.h"

class mp4_sequence_to_time : public mp4_playlist {
	public:
		mp4_sequence_to_time(uint32_t);
		~mp4_sequence_to_time() override;

	public:
		void execute(std::shared_ptr<mp4_abstract_box>) override;

		uint64_t multi_track_segment_time(std::shared_ptr<mp4_abstract_box>, uint32_t);
		uint64_t single_track_segment_time(std::shared_ptr<mp4_abstract_box>, uint32_t, uint32_t);
};
