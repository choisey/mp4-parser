/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_single_track_segment_by_sequence_number.h"
#include "mp4_sequence_to_time.h"
#include <assert.h>

MP4SingleTrackSegmentBySequenceNumber::MP4SingleTrackSegmentBySequenceNumber(uint32_t track_id, uint32_t seq, uint32_t duration)
	: MP4SingleTrackSegment(track_id, 0, duration)
	, _segment_sequence(seq)
{
}

MP4SingleTrackSegmentBySequenceNumber::~MP4SingleTrackSegmentBySequenceNumber()
{
}

void MP4SingleTrackSegmentBySequenceNumber::execute(std::shared_ptr<mp4_abstract_box> mp4)
{
	assert( MP4FILE == mp4->head().boxtype );

	MP4SequenceToTime seq(_segment_duration_in_second);
	_segment_time_in_timescale =  seq.single_track_segment_time(mp4, _track_id, _segment_sequence);

#ifdef _DEBUG
	fprintf(stderr, "seq# %u -> time %lu (in timescale)\n", _segment_sequence, _segment_time_in_timescale);
#endif

	MP4SingleTrackSegment::execute(mp4);
}
