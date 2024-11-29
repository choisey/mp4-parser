/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "MP4SingleTrackSegment.h"

class MP4SingleTrackSegmentBySequenceNumber : public MP4SingleTrackSegment {
	public:
		MP4SingleTrackSegmentBySequenceNumber(uint32_t, uint32_t, uint32_t);
		virtual ~MP4SingleTrackSegmentBySequenceNumber();

	protected:
		uint32_t _segment_sequence;

	public:
		virtual void execute(std::shared_ptr<MP4AbstractBox>);
};
