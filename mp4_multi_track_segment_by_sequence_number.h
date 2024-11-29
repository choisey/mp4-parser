/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_multi_track_segment.h"
#include "mp4_playlist.h"

class MP4MultiTrackSegmentBySequenceNumber : public MP4MultiTrackSegment {
	public:
		MP4MultiTrackSegmentBySequenceNumber(uint32_t, uint32_t);
		virtual ~MP4MultiTrackSegmentBySequenceNumber();

	protected:
		uint32_t _segment_sequence;

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};