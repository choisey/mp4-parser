/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_segment.h"

class mp4_single_track_segment : public mp4_segment {
	public:
		mp4_single_track_segment(uint32_t, uint64_t, uint32_t);
		~mp4_single_track_segment() override;

	protected:
		uint32_t _track_id;
		uint64_t _segment_time_in_timescale;
		uint32_t _segment_duration_in_second;

	public:
		void execute(std::shared_ptr<mp4_abstract_box>) override;
};
