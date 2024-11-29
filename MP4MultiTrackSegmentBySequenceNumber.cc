/*
 * Copyright (c) Seungyeob Choi
 */

#include "MP4MultiTrackSegmentBySequenceNumber.h"
#include "MP4SequenceToTime.h"
#include "mp4.h"
#include <assert.h>

#define SLASH '/'

MP4MultiTrackSegmentBySequenceNumber::MP4MultiTrackSegmentBySequenceNumber(uint32_t seq, uint32_t duration)
	: MP4MultiTrackSegment(0, duration)
	, _segment_sequence(seq)
{
}

MP4MultiTrackSegmentBySequenceNumber::~MP4MultiTrackSegmentBySequenceNumber()
{
}

void MP4MultiTrackSegmentBySequenceNumber::execute(std::shared_ptr<mp4_abstract_box> mp4)
{
	assert( MP4FILE == mp4->head().boxtype );

	MP4SequenceToTime seq(_segment_duration_in_second);
	_segment_time_in_millisecond =  seq.multi_track_segment_time(mp4, _segment_sequence);

#ifdef _DEBUG
	fprintf(stderr, "seq# %u -> time %lu(ms)\n", _segment_sequence, _segment_time_in_millisecond);
#endif

	MP4MultiTrackSegment::execute(mp4);
}
