/*
 * Copyright (c) Seungyeob Choi
 */

#include "MP4MultiTrackSegment.h"
#include "mp4.h"
#include <assert.h>
#include <stdio.h>

#define INC0306664

MP4MultiTrackSegment::MP4MultiTrackSegment(uint64_t time, uint32_t duration)
	: _segment_time_in_millisecond(time)
	, _segment_duration_in_second(duration)
{
}

MP4MultiTrackSegment::~MP4MultiTrackSegment()
{
}

bool MP4MultiTrackSegment::mp4_segment_boundary(
		std::shared_ptr<MP4AbstractBox> box,
		std::pair<uint64_t, uint64_t>& segment_time_range)
{
	assert( MP4FILE == box->head().boxtype );

	for ( auto trak: select(box, TRAK) ) {
		const auto& mdhd = select<MediaHeaderBox>(trak);
		assert( 1 == mdhd.size() );
		if ( mdhd.empty() ) continue;

		const auto& hdlr = select<HandlerBox>(trak);
		assert( 1 == hdlr.size() );
		if ( hdlr.empty() ) continue;

		if ( HandlerBox::VIDEO != hdlr[0]->data().handler_type ) {
			continue;
		}

		const auto& stss = select<SyncSampleBox>(trak);
		if ( stss.empty() ) {
			continue;
		}

		const auto& stts = select<TimeToSampleBox>(trak);
		assert( 1 == stts.size() );
		if ( stts.empty() ) {
			continue;
		}

		struct DeltaBlock {
			uint64_t first_sample_time;
			uint32_t first_sample;
			uint32_t delta;
		};

		std::map<uint32_t, DeltaBlock> sample_to_delta; // to be looked up using lower_bound()
		uint32_t sample_count = 0;
		uint64_t time_offset = 0;

		for ( const auto& d: stts[0]->data().entries ) {
			sample_to_delta[ sample_count + d.sample_count ] = {
				time_offset,
				sample_count + 1,
				d.sample_delta };
			sample_count += d.sample_count;
			time_offset += (uint64_t) d.sample_count * d.sample_delta;
		}

		sample_to_delta[MAX_32BIT] = {
			time_offset,
			sample_count + 1,
			0 };

#ifdef _DEBUG
		std::map<uint32_t, std::vector<uint64_t>> sample_to_delta_dump;
		for ( auto si: sample_to_delta ) {
			std::vector<uint64_t> d;
			d.push_back(si.second.first_sample_time);
			d.push_back(si.second.first_sample);
			d.push_back(si.second.delta);
			sample_to_delta_dump[si.first] = d;
		}
		std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":{ last_sample->(first_sample_time, first_sample, sample_delta) }=" << sample_to_delta_dump << std::endl;
#endif

		std::map<uint64_t, uint32_t> time_to_ss;
		for ( auto ss: stss[0]->data().sample_numbers ) {
			auto sdi = sample_to_delta.lower_bound(ss);
			assert( sample_to_delta.end() != sdi );
			if ( sample_to_delta.end() == sdi ) {
				break;
			}

			uint64_t t = sdi->second.first_sample_time + (uint64_t) ( ss - sdi->second.first_sample ) * sdi->second.delta;
			time_to_ss[ t ] = ss;
		}

#ifdef _DEBUG
		std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":{ sample_time->sample }=" << time_to_ss << std::endl;
#endif

		std::pair<uint64_t, uint64_t> tt;
		auto tsi = time_to_ss.lower_bound( _segment_time_in_millisecond * mdhd[0]->data().timescale / 1000 );
		if ( time_to_ss.end() != tsi ) {
			tt.first = tsi->first;
		}
		else {
			tt.first = time_offset;
		}

		uint64_t segment_target_duration = _segment_duration_in_second * mdhd[0]->data().timescale;
		tsi = time_to_ss.lower_bound( ( tt.first / segment_target_duration + 1 ) * segment_target_duration );
		if ( time_to_ss.end() != tsi ) {
			tt.second = tsi->first;
		}
		else {
			tt.second = time_offset;
		}

		// special case: duration of last segment < 1 second
		if ( tt.second != time_offset && time_offset < tt.second + mdhd[0]->data().timescale * MIN_DURATION_OF_LAST_SEGMENT ) {
			tt.second = time_offset;
		}

		segment_time_range.first = tt.first * 1000 / mdhd[0]->data().timescale;
		segment_time_range.second = tt.second * 1000 / mdhd[0]->data().timescale;

		return true;
	}

	for ( auto trak: select(box, TRAK) ) {
		const auto& mdhd = select<MediaHeaderBox>(trak);
		assert( 1 == mdhd.size() );
		if ( mdhd.empty() ) continue;

		const auto& stts = select<TimeToSampleBox>(trak);
		assert( 1 == stts.size() );
		if ( stts.empty() ) {
			continue;
		}

		struct DeltaBlock {
			uint64_t first_sample_time;
			uint32_t first_sample;
			uint32_t delta;
		};

		std::map<uint64_t, DeltaBlock> time_to_delta; // to be looked up using upper_bound()
		uint32_t sample_count = 0;
		uint64_t time_offset = 0;

		for ( const auto& d: stts[0]->data().entries ) {
			uint64_t next_time_offset = time_offset + (uint64_t) d.sample_count * d.sample_delta;
			time_to_delta[ next_time_offset ] = {
				time_offset,
				sample_count + 1,
				d.sample_delta };
			sample_count += d.sample_count;
			time_offset = next_time_offset;
		}

		time_to_delta[MAX_64BIT] = {
			time_offset,
			sample_count + 1,
			0 };

#ifdef _DEBUG
		std::map<uint64_t, std::vector<uint64_t>> time_to_delta_dump;
		for ( auto si: time_to_delta ) {
			std::vector<uint64_t> d;
			d.push_back(si.second.first_sample_time);
			d.push_back(si.second.first_sample);
			d.push_back(si.second.delta);
			time_to_delta_dump[si.first] = d;
		}
		std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":{ time->(first_sample_time, first_sample, sample_delta) }=" << time_to_delta_dump << std::endl;
#endif

		std::pair<uint64_t, uint64_t> tt;
		uint64_t time = _segment_time_in_millisecond * mdhd[0]->data().timescale / 1000;
		std::pair<uint32_t, uint32_t> ss;

		auto tdi = time_to_delta.upper_bound( time );
		assert( time_to_delta.end() != tdi );
		assert( tdi->second.first_sample_time <= time_offset );

		if ( tdi->second.first_sample_time < time_offset ) {
			uint32_t sc = ( time - tdi->second.first_sample_time + ( tdi->second.delta - 1 ) ) / tdi->second.delta;
			tt.first = tdi->second.first_sample_time + (uint64_t) sc * tdi->second.delta;
		}
		else {
			tt.first = time_offset;
		}

		uint64_t segment_target_duration = _segment_duration_in_second * mdhd[0]->data().timescale;
		uint64_t upper_bound = ( time / segment_target_duration + 1 ) * segment_target_duration;
		tdi = time_to_delta.upper_bound( upper_bound );
		assert( time_to_delta.end() != tdi );
		assert( tdi->second.first_sample_time <= time_offset );

		if ( tdi->second.first_sample_time < time_offset ) {
			uint32_t sc = ( upper_bound - tdi->second.first_sample_time + ( tdi->second.delta - 1 ) ) / tdi->second.delta;
			tt.second = tdi->second.first_sample_time + (uint64_t) sc * tdi->second.delta;
		}
		else {
			tt.second = time_offset;
		}

		// special case: duration of last segment < 1 second
		if ( tt.second != time_offset && time_offset < tt.second + mdhd[0]->data().timescale * MIN_DURATION_OF_LAST_SEGMENT ) {
			tt.second = time_offset;
		}

		segment_time_range.first = tt.first * 1000 / mdhd[0]->data().timescale;
		segment_time_range.second = tt.second * 1000 / mdhd[0]->data().timescale;

		return true;
	}

	return false;
}

void MP4MultiTrackSegment::make_empty(std::shared_ptr<MP4AbstractBox> box)
{
	assert( TRAK == box->head().boxtype );

	for ( auto stsz: select<SampleSizeBox>(box) ) {
		stsz->data().sample_size = 0;
		stsz->data().entry_sizes.clear();
	}

	for ( auto stsc: select<SampleToChunkBox>(box) ) {
		stsc->data().entries.clear();
	}

	for ( auto ctts: select<CompositionOffsetBox>(box) ) {
		ctts->data().entries.clear();
	}

	for ( auto stco: select<ChunkOffsetBox>(box) ) {
		stco->data().chunk_offsets.clear();
	}

	for ( auto co64: select<ChunkLargeOffsetBox>(box) ) {
		co64->data().chunk_offsets.clear();
	}

	for ( auto stts: select<TimeToSampleBox>(box) ) {
		stts->data().entries.clear();
	}

	for ( auto stss: select<SyncSampleBox>(box) ) {
		stss->data().sample_numbers.clear();
	}

	for ( auto elst: select<EditListBox>(box) ) {
		elst->data().entries.clear();
	}

	for ( auto mdhd: select<MediaHeaderBox>(box) ) {
		mdhd->data().duration = 0;
	}

	for ( auto tkhd: select<TrackHeaderBox>(box) ) {
		tkhd->data().duration = 0;
	}
}

void MP4MultiTrackSegment::execute(std::shared_ptr<MP4AbstractBox> mp4)
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

	// get the exact boundary from a 1) video, or 2) audio track

	std::pair<uint64_t, uint64_t> mp4_segment_time_range(0, 0);
	if ( !mp4_segment_boundary(
				mp4,
				mp4_segment_time_range) ) return;

#ifdef _DEBUG
	std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":segment_time_range=" << mp4_segment_time_range << std::endl;
#endif

	// mvhd

	const auto& mvhd = select<MovieHeaderBox>(mp4);
	assert( 1 <= mvhd.size() ); // we saw an erraneous mp4 that has two mvhd
	if ( mvhd.empty() ) return;
	mvhd[0]->data().duration = 0;
	uint32_t movie_timescale = mvhd[0]->data().timescale;

	// divide both video/audio segments using video boundary

	for ( auto trak: select(mp4, TRAK) ) {
		const auto& tkhd = select<TrackHeaderBox>(trak);
		assert( 1 == tkhd.size() );
		if ( tkhd.empty() ) continue;

		const auto& hdlr = select<HandlerBox>(trak);
		assert( 1 == hdlr.size() );
		if ( hdlr.empty() ) continue;

		if ( HandlerBox::VIDEO != hdlr[0]->data().handler_type
				&& HandlerBox::AUDIO != hdlr[0]->data().handler_type ) {
			// skip if the track is neither video nor audio
			// INC0335886
			//trak->remove();
			// we shouldn't remove metadata tracks. we just make them empty.
			// INC0361617
			make_empty(trak);
			continue;
		}

		const auto& mdia = select(trak, MDIA);
		assert( 1 == mdia.size() );
		if ( mdia.empty() ) continue;

		const auto& mdhd = select<MediaHeaderBox>(mdia[0]);
		assert( 1 == mdhd.size() );
		if ( mdhd.empty() ) continue;
		uint32_t media_timescale = mdhd[0]->data().timescale;

		const auto& minf = select(trak, MINF);
		assert( 1 == minf.size() );
		if ( minf.empty() ) continue;

		//std::pair<uint64_t, uint64_t> mdia_segment_time_range;
		//std::pair<uint32_t, uint32_t> mdia_segment_sample_range;
		uint64_t duration;

		if ( divide(minf[0], media_timescale,
					std::make_pair(
						mp4_segment_time_range.first * media_timescale / 1000,
						mp4_segment_time_range.second * media_timescale / 1000),
					duration) ) {

			mdhd[0]->data().duration = duration;
			tkhd[0]->data().duration = duration * movie_timescale / media_timescale;

			if ( mvhd[0]->data().duration < tkhd[0]->data().duration ) {
				mvhd[0]->data().duration = tkhd[0]->data().duration;
			}

			// create/update edts
			update(
					trak,
					movie_timescale,
					media_timescale,
					mp4_segment_time_range.first * media_timescale / 1000,
					duration);
		}
		else {
			trak->remove();
		}
	}

	// uddate mdat
	update(mp4);

#ifdef INC0306664
	// INC0306664: Chrome Cast hack
	for ( auto elst: select<EditListBox>(mp4) ) {
		for ( auto& edit: elst->data().entries ) {
			if ( 0 < edit.media_time ) {
				edit.media_time = 0;
			}
		}
	}
#endif
}
