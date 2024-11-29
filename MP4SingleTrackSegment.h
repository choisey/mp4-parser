/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "MP4Segment.h"

class MP4SingleTrackSegment : public MP4Segment {
	public:
		MP4SingleTrackSegment(uint32_t, uint64_t, uint32_t);
		virtual ~MP4SingleTrackSegment();

	protected:
		uint32_t _track_id;
		uint64_t _segment_time_in_timescale;
		uint32_t _segment_duration_in_second;

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};
