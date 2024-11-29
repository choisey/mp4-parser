/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "MP4MultiTrackSegment.h"
#include "MP4Playlist.h"

class MP4MultiTrackSegmentBySequenceNumber : public MP4MultiTrackSegment {
	public:
		MP4MultiTrackSegmentBySequenceNumber(uint32_t, uint32_t);
		virtual ~MP4MultiTrackSegmentBySequenceNumber();

	protected:
		uint32_t _segment_sequence;

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};
