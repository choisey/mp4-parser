/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "MP4Segment.h"

class MP4MultiTrackSegment : public MP4Segment {
	public:
		MP4MultiTrackSegment(uint64_t, uint32_t);
		virtual ~MP4MultiTrackSegment();

	protected:
		uint64_t _segment_time_in_millisecond;
		uint32_t _segment_duration_in_second;

	protected:
		bool mp4_segment_boundary(
				std::shared_ptr<mp4_abstract_box>,
				std::pair<uint64_t, uint64_t>&);

		void make_empty(std::shared_ptr<mp4_abstract_box>);

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};
