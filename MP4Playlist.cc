#include "MP4Playlist.h"
#include "MP4.h"
#include <assert.h>
#include <stdio.h>

// MP4Playlist

MP4Playlist::MP4Playlist(uint32_t duration)
	: _segment_duration_in_second(duration)
{
}

MP4Playlist::~MP4Playlist()
{
}

bool MP4Playlist::minf_with_stss_to_segments(
		std::shared_ptr<MP4AbstractBox> box,
		uint64_t segment_duration, uint32_t timescale,
		const std::vector<uint32_t>& sync_samples,
		std::vector<std::pair<uint64_t, uint64_t>>& segments)
{
	const auto& stts = select<TimeToSampleBox>(box);
	assert( 1 == stts.size() );
	if ( 1 != stts.size() ) {
		return false;
	}

	struct TimeAndSampleOfNewDelta {
		uint64_t first_sample_time;
		uint32_t first_sample;
		uint32_t delta;
	};

	std::map<uint32_t, TimeAndSampleOfNewDelta> sample_to_delta; // to be looked up using lower_bound()
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
	std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":{ sample->(first_sample_time, first_sample, sample_delta) }=" << sample_to_delta_dump << std::endl;
#endif

	std::map<uint64_t, uint32_t> time_to_ss;
	for ( auto ss: sync_samples ) {
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

	for ( uint64_t cur_t = 0; cur_t < time_offset; ) {
		auto tsi = time_to_ss.lower_bound( ( cur_t / segment_duration + 1 ) * segment_duration );
		uint64_t next_t = ( time_to_ss.end() != tsi ) ? tsi->first : time_offset;
		segments.push_back(std::make_pair(cur_t, next_t - cur_t));
		cur_t = next_t;
	}

	return true;
}

bool MP4Playlist::minf_without_stss_to_segments(
		std::shared_ptr<MP4AbstractBox> box,
		uint64_t segment_duration, uint32_t timescale,
		std::vector<std::pair<uint64_t, uint64_t>>& segments)
{
	const auto& stts = select<TimeToSampleBox>(box);
	assert( 1 == stts.size() );
	if ( 1 != stts.size() ) {
		return false;
	}

	struct TimeOfNewDelta {
		uint64_t first_sample_time;
		uint32_t delta;
	};

	std::map<uint64_t, TimeOfNewDelta> time_to_delta; // to be looked up using upper_bound()
	uint32_t sample_count = 0;
	uint64_t time_offset = 0;

	for ( const auto& d: stts[0]->data().entries ) {
		uint64_t next_time_offset = time_offset + (uint64_t) d.sample_count * d.sample_delta;
		time_to_delta[ next_time_offset ] = {
			time_offset,
			d.sample_delta };
		time_offset = next_time_offset;
	}

	time_to_delta[MAX_64BIT] = { time_offset, 0 };

#ifdef _DEBUG
	std::map<uint64_t, std::vector<uint64_t>> time_to_delta_dump;
	for ( auto si: time_to_delta ) {
		std::vector<uint64_t> d;
		d.push_back(si.second.first_sample_time);
		d.push_back(si.second.delta);
		time_to_delta_dump[si.first] = d;
	}
	std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":{ time->(first_sample_time, sample_delta) }=" << time_to_delta_dump << std::endl;
#endif

	for ( uint64_t cur_t = 0; cur_t < time_offset; ) {
		uint64_t next_target_t = ( cur_t / segment_duration + 1 ) * segment_duration;
		auto tdi = time_to_delta.upper_bound( next_target_t );
		assert( time_to_delta.end() != tdi );
		assert( tdi->second.first_sample_time <= time_offset );

		uint64_t next_t = ( tdi->second.first_sample_time < time_offset )
			? tdi->second.first_sample_time + (uint64_t) ( ( next_target_t - tdi->second.first_sample_time + ( tdi->second.delta - 1 ) ) / tdi->second.delta ) * tdi->second.delta
			: tdi->second.first_sample_time;
		segments.push_back(std::make_pair(cur_t, next_t - cur_t));
		cur_t = next_t;
	}

	return true;
}

bool MP4Playlist::minf_to_segments(
		std::shared_ptr<MP4AbstractBox> minf,
		uint32_t timescale,
		std::vector<std::pair<uint64_t, uint64_t>>& segments)
{
	assert( MINF == minf->head().boxtype );

	const auto& stss = select<SyncSampleBox>(minf);
	if ( !stss.empty() ) {
		assert( 1 == stss.size() );
		if ( !minf_with_stss_to_segments(
				minf,
				_segment_duration_in_second * timescale,
				timescale,
				stss[0]->data().sample_numbers,
				segments) ) return false;
	}
	else {
		if ( !minf_without_stss_to_segments(
				minf,
				_segment_duration_in_second * timescale,
				timescale,
				segments) ) return false;
	}

	// special case: duration of last segment < 1 second
	// then merge the last segment to the previous one.
	if ( 1 < segments.size() ) {
		auto last = segments.back();
		if ( last.second < timescale * MIN_DURATION_OF_LAST_SEGMENT ) {
			segments.pop_back();
			segments.back().second += last.second;
		}
	}

	return true;
}

void MP4Playlist::execute(std::shared_ptr<MP4AbstractBox> mp4)
{
	/*
		+------+       +------+
		| TRAK | --+-- | TKHD |
		+------+   |   +------+
		           |
		           |   +------+       +------+
		           +-- | MDIA | --+-- | MDHD |
		               +------+   |   +------+
		                          |
		                          |   +------+
		                          +-- | HDLR |
		                          |   +------+
		                          |
		                          |   +------+       +------+       +------+
		                          +-- | MINF | --+-- | STBL | --+-- | STSS |
		                              +------+       +------+       +------+
	*/

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

		if ( HandlerBox::VIDEO != hdlr[0]->data().handler_type
				&& HandlerBox::AUDIO != hdlr[0]->data().handler_type ) continue;

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

#ifdef _DEBUG
		std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":segments (start, duration in millisecond)=";
		if ( !segments.empty() ) {
			std::vector<std::pair<uint64_t, uint64_t>> segmentdump;
			for ( auto segment: segments ) {
				segmentdump.push_back(std::make_pair(
							segment.first * 1000 / timescale,
							segment.second * 1000 / timescale));
			}
			std::cerr << segmentdump;
		}
		std::cerr << std::endl;
#endif

		printf("%u:%c%c%c%c/%u ", tkhd[0]->data().track_ID,
			(char) ( hdlr[0]->data().handler_type / 0x1000000 ),
			(char) ( ( hdlr[0]->data().handler_type / 0x10000 ) & 0xff ),
			(char) ( ( hdlr[0]->data().handler_type / 0x100 ) & 0xff ),
			(char) ( hdlr[0]->data().handler_type & 0xff ),
			timescale);
		if ( !segments.empty() ) {
			for ( auto segment: segments ) {
				printf("%lu ", segment.second);
			}
		}
		printf("\n");
	}
}
