/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_multi_track_segment_by_sequence_number.h"
#include "mp4_sequence_to_time.h"
#include "mp4.h"
#include <assert.h>

#define SLASH '/'

mp4_multi_track_segment_by_sequence_number::mp4_multi_track_segment_by_sequence_number(uint32_t seq, uint32_t duration)
	: mp4_multi_track_segment(0, duration)
	, _segment_sequence(seq)
{
}

mp4_multi_track_segment_by_sequence_number::~mp4_multi_track_segment_by_sequence_number()
{
}

void mp4_multi_track_segment_by_sequence_number::execute(std::shared_ptr<mp4_abstract_box> mp4)
{
	assert( MP4FILE == mp4->head().boxtype );

	mp4_sequence_to_time seq(_segment_duration_in_second);
	_segment_time_in_millisecond =  seq.multi_track_segment_time(mp4, _segment_sequence);

#ifdef _DEBUG
	fprintf(stderr, "seq# %u -> time %lu(ms)\n", _segment_sequence, _segment_time_in_millisecond);
#endif

	mp4_multi_track_segment::execute(mp4);
}
