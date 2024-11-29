/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_single_track_segment.h"

class mp4_single_track_segment_by_sequence_number : public mp4_single_track_segment {
	public:
		mp4_single_track_segment_by_sequence_number(uint32_t, uint32_t, uint32_t);
		virtual ~mp4_single_track_segment_by_sequence_number();

	protected:
		uint32_t _segment_sequence;

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};
