/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_segment.h"

class mp4_multi_track_segment : public mp4_segment {
	public:
		mp4_multi_track_segment(uint64_t, uint32_t);
		~mp4_multi_track_segment() override;

	protected:
		uint64_t _segment_time_in_millisecond;
		uint32_t _segment_duration_in_second;

	protected:
		bool mp4_segment_boundary(
				std::shared_ptr<mp4_abstract_box>,
				std::pair<uint64_t, uint64_t>&);

		void make_empty(std::shared_ptr<mp4_abstract_box>);

	public:
		void execute(std::shared_ptr<mp4_abstract_box>) override;
};
