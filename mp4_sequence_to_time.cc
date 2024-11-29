/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_sequence_to_time.h"
#include "mp4.h"
#include <assert.h>

#define SLASH '/'

mp4_sequence_to_time::mp4_sequence_to_time(uint32_t duration)
	: mp4_playlist(duration)
{
}

mp4_sequence_to_time::~mp4_sequence_to_time()
{
}

uint64_t mp4_sequence_to_time::multi_track_segment_time(std::shared_ptr<mp4_abstract_box> mp4, uint32_t segment_seq)
{
	const uint32_t track_precedence[] = {
		HandlerBox::VIDEO,
		HandlerBox::AUDIO
	};

	for ( uint32_t handler_type: track_precedence ) {
		for ( auto trak: select(mp4, TRAK) ) {
			const auto& tkhd = select<TrackHeaderBox>(trak);
			assert( 1 == tkhd.size() );
			if ( tkhd.empty() ) continue;

			const auto& mdhd = select<MediaHeaderBox>(trak);
			assert( 1 == mdhd.size() );
			if ( mdhd.empty() ) {
				continue;
			}

			uint32_t timescale = mdhd[0]->data().timescale;

			const auto& hdlr = select<HandlerBox>(trak);
			assert( 1 == hdlr.size() );
			if ( hdlr.empty() ) continue;

			if ( hdlr[0]->data().handler_type != handler_type ) {
				// skip if the track is not the right type
				continue;
			}

			const auto& mdia = select(trak, MDIA);
			assert( 1 == mdia.size() );
			if ( mdia.empty() ) continue;

			const auto& minf = select(mdia[0], MINF);
			assert( 1 == minf.size() );
			if ( minf.empty() ) continue;

			std::vector<std::pair<uint64_t, uint64_t>> segments;

			minf_to_segments(
					minf[0], timescale,
					segments);

			uint64_t time = 0;
			if ( !segments.empty() ) {
				uint64_t max_duration = 0;
				for ( auto segment: segments ) {
					if ( max_duration < segment.second ) {
						max_duration = segment.second;
					}
				}

				for ( auto segment: segments ) {
					if ( segment_seq-- <= 1 ) {
						return  time * 1000 / timescale;
					}
					//printf("%s+%lu.ts\n", file.c_str(), time * 1000 / timescale);
					time += segment.second;
				}
			}
		}
	}

	return 0;
}

uint64_t mp4_sequence_to_time::single_track_segment_time(std::shared_ptr<mp4_abstract_box> mp4, uint32_t track_id, uint32_t segment_seq)
{
	for ( auto trak: select(mp4, TRAK) ) {
		const auto& tkhd = select<TrackHeaderBox>(trak);
		assert( 1 == tkhd.size() );
		if ( tkhd.empty() ) continue;
		if ( tkhd[0]->data().track_ID != track_id ) continue;

		const auto& mdhd = select<MediaHeaderBox>(trak);
		assert( 1 == mdhd.size() );
		if ( mdhd.empty() ) {
			continue;
		}

		uint32_t timescale = mdhd[0]->data().timescale;

		const auto& hdlr = select<HandlerBox>(trak);
		assert( 1 == hdlr.size() );
		if ( hdlr.empty() ) continue;

		const auto& mdia = select(trak, MDIA);
		assert( 1 == mdia.size() );
		if ( mdia.empty() ) continue;

		const auto& minf = select(mdia[0], MINF);
		assert( 1 == minf.size() );
		if ( minf.empty() ) continue;

		std::vector<std::pair<uint64_t, uint64_t>> segments;

		minf_to_segments(
				minf[0], timescale,
				segments);

		if ( !segments.empty() ) {
			uint64_t max_duration = 0;
			for ( auto segment: segments ) {
				if ( max_duration < segment.second ) {
					max_duration = segment.second;
				}
			}

			uint64_t time = 0;
			for ( auto segment: segments ) {
				if ( segment_seq-- <= 1 ) {
					return  time;
				}
				//printf("#EXTINF:%0.5f,\n", (double) segment.second / timescale);
				//printf("%u+%lu.mp4\n", tkhd[0]->data().track_ID, time);
				time += segment.second;
			}
		}
	}

	return 0;
}

void mp4_sequence_to_time::execute(std::shared_ptr<mp4_abstract_box> mp4)
{
	// This is not supposed to be executed.
	assert( false );
}
