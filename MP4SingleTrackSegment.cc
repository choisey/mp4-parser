/*
 * Copyright (c) Seungyeob Choi
 */

#include "MP4SingleTrackSegment.h"
#include "mp4.h"
#include <assert.h>

MP4SingleTrackSegment::MP4SingleTrackSegment(uint32_t track_id, uint64_t time, uint32_t duration)
	: _track_id(track_id)
	, _segment_time_in_timescale(time)
	, _segment_duration_in_second(duration)
{
}

MP4SingleTrackSegment::~MP4SingleTrackSegment()
{
}

void MP4SingleTrackSegment::execute(std::shared_ptr<mp4_abstract_box> mp4)
{
	assert( MP4FILE == mp4->head().boxtype );

	//	+------+       +------+
	//	| MVHD | --+-- | MVHD |
	//	+------+   |   +------+
	//		   |
	//		   |   +------+       +------+
	//		   +-- | TRAK | --+-- | TKHD |
	//		       +------+   |   +------+
	//			          |
	//			          |   +------+       +------+
	//			          +-- | MDIA | --+-- | MDHD |
	//			              +------+   |   +------+
	//					         |
	//					         |   +------+
	//					         +-- | HDLR |
	//					         |   +------+
	//					         |
	//					         |   +------+     +------+     +------+
	//					         +-- | MINF | --- | STBL | --- | STSS |
	//					             +------+     +------+     +------+

	// mvhd

	const auto& mvhd = select<MovieHeaderBox>(mp4);
	assert( 1 == mvhd.size() );
	if ( mvhd.empty() ) return;
	uint32_t movie_timescale = mvhd[0]->data().timescale;

	for ( auto trak: select(mp4, TRAK) ) {
		const auto& tkhd = select<TrackHeaderBox>(trak);
		assert( 1 == tkhd.size() );
		if ( tkhd.empty() || tkhd[0]->data().track_ID != _track_id ) {
			trak->remove();
			continue;
		}

		const auto& mdia = select(trak, MDIA);
		assert( 1 == mdia.size() );
		if ( mdia.empty() ) continue;

		const auto& mdhd = select<MediaHeaderBox>(mdia[0]);
		assert( 1 == mdhd.size() );
		if ( mdhd.empty() ) continue;

		const auto& minf = select(mdia[0], MINF);
		assert( 1 == minf.size() );
		if ( minf.empty() ) continue;

		uint32_t media_timescale = mdhd[0]->data().timescale;
		uint64_t sd = (uint64_t) _segment_duration_in_second * media_timescale;
		uint64_t ub = ( _segment_time_in_timescale / sd + 1 ) * sd;

		uint64_t duration;

		if ( divide(minf[0], media_timescale,
					std::make_pair(
						_segment_time_in_timescale,
						ub),
					duration) ) {

			mdhd[0]->data().duration = duration;
			tkhd[0]->data().duration = duration * movie_timescale / media_timescale;
			mvhd[0]->data().duration = tkhd[0]->data().duration;

			// create/update edts
			update(
					trak,
					movie_timescale,
					media_timescale,
					_segment_time_in_timescale,
					duration);
		}
		else {
			trak->remove();
		}
	}

	// update mdat
	update(mp4);
}
