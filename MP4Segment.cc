/*
 * Copyright (c) Seungyeob Choi
 */

#include "MP4Segment.h"
#include "mp4.h"
#include <assert.h>
#include <stdio.h>

MP4Segment::MP4Segment()
{
}

MP4Segment::~MP4Segment()
{
}

bool MP4Segment::mdia_segment_time_and_sample_range(
		std::shared_ptr<mp4_abstract_box> box, uint32_t timescale,
		std::pair<uint64_t, uint64_t> segment_time_target_range,
		std::pair<uint64_t, uint64_t>& segment_time_range,
		std::pair<uint32_t, uint32_t>& segment_sample_range)
{
	assert( MINF == box->head().boxtype );

	const auto& stss = select<SyncSampleBox>(box);
	if ( !stss.empty() ) {
		const auto& stts = select<TimeToSampleBox>(box);
		assert( 1 == stts.size() );
		if ( 1 != stts.size() ) {
			return false;
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
		std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":{ sample->(first_sample_time, first_sample, sample_delta) }=" << sample_to_delta_dump << std::endl;
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

		auto tsi = time_to_ss.lower_bound( segment_time_target_range.first );
		if ( time_to_ss.end() != tsi ) {
			segment_time_range.first = tsi->first;
			segment_sample_range.first = tsi->second;
		}
		else {
			segment_time_range.first = time_offset;
			segment_sample_range.first = sample_count + 1;
		}

		tsi = time_to_ss.lower_bound( segment_time_target_range.second );

		if ( time_to_ss.end() != tsi ) {
			segment_time_range.second = tsi->first;
			segment_sample_range.second = tsi->second - 1;
		}
		else {
			segment_time_range.second = time_offset;
			segment_sample_range.second = ( sample_count + 1 == segment_sample_range.first ) ? sample_count + 1 : sample_count;
		}

		// special case: duration of last segment < 1 second
		if ( segment_time_range.second != time_offset && time_offset < segment_time_range.second + (uint64_t) timescale * MIN_DURATION_OF_LAST_SEGMENT ) {
			segment_time_range.second = time_offset;
			segment_sample_range.second = sample_count;
		}
	}
	else {
		const auto& stts = select<TimeToSampleBox>(box);
		assert( 1 == stts.size() );
		if ( 1 != stts.size() ) {
			return false;
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

		auto tdi = time_to_delta.upper_bound( segment_time_target_range.first );
		assert( time_to_delta.end() != tdi );
		assert( tdi->second.first_sample_time <= time_offset );

		if ( tdi->second.first_sample_time < time_offset ) {
			uint32_t sc = ( segment_time_target_range.first - tdi->second.first_sample_time + ( tdi->second.delta - 1 ) ) / tdi->second.delta;
			segment_time_range.first = tdi->second.first_sample_time + (uint64_t) sc * tdi->second.delta;
			segment_sample_range.first = tdi->second.first_sample + sc;
		}
		else {
			segment_time_range.first = time_offset;
			segment_sample_range.first = sample_count + 1;
		}

		//uint64_t segment_target_duration = _segment_duration_in_second * timescale;
		//uint64_t upper_bound = ( _segment_time_in_timescale / segment_target_duration + 1 ) * segment_target_duration;
		//tdi = time_to_delta.upper_bound( upper_bound );
		tdi = time_to_delta.upper_bound( segment_time_target_range.second );
		assert( time_to_delta.end() != tdi );
		assert( tdi->second.first_sample_time <= time_offset );

		if ( tdi->second.first_sample_time < time_offset ) {
			uint32_t sc = ( segment_time_target_range.second - tdi->second.first_sample_time + ( tdi->second.delta - 1 ) ) / tdi->second.delta;
			segment_time_range.second = tdi->second.first_sample_time + (uint64_t) sc * tdi->second.delta;
			segment_sample_range.second = tdi->second.first_sample + sc - 1;
		}
		else {
			segment_time_range.second = time_offset;
			segment_sample_range.second = ( sample_count + 1 == segment_sample_range.first ) ? sample_count + 1 : sample_count;
		}

		// special case: duration of last segment < 1 second
		if ( segment_time_range.second != time_offset && time_offset < segment_time_range.second + (uint64_t) timescale * MIN_DURATION_OF_LAST_SEGMENT ) {
			segment_time_range.second = time_offset;
			segment_sample_range.second = sample_count;
		}
	}

#ifdef _DEBUG
	std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":segment_boundary time=" << segment_time_range
		<< "=(" << (double) segment_time_range.first / timescale << ',' << (double) segment_time_range.second / timescale
		<< "), sample=" << segment_sample_range << std::endl;
#endif

	return true;
}

bool MP4Segment::sample_to_chunk(
		std::shared_ptr<mp4_abstract_box> box,
		std::pair<uint32_t, uint32_t> ss,
		std::vector<Chunk>& chunks)
{
	class ChunkSample {
		private:
			std::vector<SampleToChunkBox::Entry>& _sc;
			uint32_t _chunk;
			uint32_t _samples_per_chunk;
			uint32_t _sample_description_index;

			std::vector<SampleToChunkBox::Entry>::iterator _sci;
			uint32_t _next_chunk_to_change_number_of_samples;

		public:
			ChunkSample(std::vector<SampleToChunkBox::Entry>& sc) : _sc(sc), _chunk(0), _samples_per_chunk(0), _sample_description_index(0) {
				_sci = _sc.begin();
				_next_chunk_to_change_number_of_samples = (*_sci).first_chunk;
			}
			~ChunkSample() {}

			ChunkSample& operator++() {
				if ( ++_chunk == _next_chunk_to_change_number_of_samples ) {
					_samples_per_chunk = (*_sci).samples_per_chunk;
					_sample_description_index = (*_sci).sample_description_index;
					_sci++;
					if ( _sci != _sc.end() ) {
						_next_chunk_to_change_number_of_samples = (*_sci).first_chunk;
					} else {
						_next_chunk_to_change_number_of_samples = 0;
					}
				}
				return *this;
			}
			uint32_t chunk() const { return _chunk; }
			uint32_t samples() const { return _samples_per_chunk; }
			uint32_t sample_description_index() const { return _sample_description_index; }
	};

	assert( MINF == box->head().boxtype );

	const auto& stsc = select<SampleToChunkBox>(box);
	assert( 1 == stsc.size() );
	assert( !stsc[0]->data().entries.empty() );
	if ( 1 != stsc.size() || stsc[0]->data().entries.empty() ) {
		// stsc is missing or empty
		return false;
	}

	const auto& stco = select<ChunkOffsetBox>(box);
	const auto& co64= select<ChunkLargeOffsetBox>(box);
	if ( 1 == stco.size() && !stco[0]->data().chunk_offsets.empty() ) {
		ChunkSample cs(stsc[0]->data().entries);
		uint32_t si = 0;
		for ( auto off: stco[0]->data().chunk_offsets ) {
			uint32_t samples_per_chunk = (++cs).samples();
			if ( ss.first <= si + samples_per_chunk ) {
				chunks.push_back( { cs.chunk(), off, si + 1, cs.samples(), cs.sample_description_index() } );
			}
			si += samples_per_chunk;
			if ( ss.second <= si ) {
				break;
			}
		}
	}
	else if ( 1 == co64.size() && !co64[0]->data().chunk_offsets.empty() ) {
		ChunkSample cs(stsc[0]->data().entries);
		uint32_t si = 0;
		for ( auto off: co64[0]->data().chunk_offsets ) {
			uint32_t samples_per_chunk = (++cs).samples();
			if ( ss.first <= si + samples_per_chunk ) {
				chunks.push_back( { cs.chunk(), off, si + 1, cs.samples(), cs.sample_description_index() } );
			}
			si += samples_per_chunk;
			if ( ss.second <= si ) {
				break;
			}
		}
	}
	else {
		// neither stco nor co64 is present
		assert( false );
		return false;
	}

#ifdef _DEBUG
	std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":samples(" << ss.first << ", " << ss.second << ")->chunks{";
	int dump_cnt = 0;
	for ( auto c : chunks ) {
		if ( 0 != dump_cnt++ ) {
			std::cerr << ",";
		}
		std::cerr << ' ' << c.offset << '(' << c.first_sample << '-' << ( c.first_sample + c.sample_count - 1 ) << ')';
	}
	std::cerr << " }" << std::endl;
#endif

	return !chunks.empty() ? true : false;
}

bool MP4Segment::minf_divide_samples_and_chunks(
		std::shared_ptr<mp4_abstract_box> minf,
		std::pair<uint32_t, uint32_t> ss,
		const std::vector<Chunk>& chunks,
		uint64_t& duration)
{
	assert( MINF == minf->head().boxtype );
	assert( 1 <= ss.first );
	assert( ss.first <= ss.second );
	assert( !chunks.empty() );

#ifdef _DEBUG
	std::vector<std::vector<uint32_t>> chunk_dump;
	for ( auto c : chunks ) {
		std::vector<uint32_t> e;
		e.push_back( c.chunk_number );
		e.push_back( (uint32_t) c.offset );
		e.push_back( c.first_sample );
		e.push_back( c.sample_count );
		e.push_back( c.sample_description_index );
		chunk_dump.push_back( e );
	}
	std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":(chunk,offset,first_sample,#samples,sdi)=" << chunk_dump << std::endl;
#endif

//	if ( 0 == ss.first || 0 == ss.second
//			|| ss.second < ss.first ) {
//		clear(minf);
//		return 0;
//	}

	// to be set by stts
	duration = 0;

	// stsc

	const auto& stsc = select<SampleToChunkBox>(minf);
	assert( 1 == stsc.size() );
	assert( !stsc[0]->data().entries.empty() );
	std::vector<SampleToChunkBox::Entry> sample_to_chunk_entries;

	uint32_t chunk = 0;
	uint32_t samples_per_chunk = 0;
	for ( auto ci = chunks.begin(); ci != chunks.end(); ci++ ) {
		uint32_t max_lb = ( (*ci).first_sample < ss.first ) ? ss.first : (*ci).first_sample;
		uint32_t min_ub = ( (*ci).first_sample + (*ci).sample_count < ss.second + 1 ) ? (*ci).first_sample + (*ci).sample_count : ss.second + 1;
		assert( max_lb < min_ub );
		++chunk;
		if ( samples_per_chunk != min_ub - max_lb ) {
			samples_per_chunk = min_ub - max_lb;
			sample_to_chunk_entries.push_back( { chunk, samples_per_chunk, (*ci).sample_description_index } );
		}
	}

#ifdef _DEBUG
	std::vector<std::vector<uint32_t>> sample_to_chunk_dump;
	for ( auto s: sample_to_chunk_entries ) {
		std::vector<uint32_t> e;
		e.push_back( s.first_chunk );
		e.push_back( s.samples_per_chunk );
		e.push_back( s.sample_description_index );
		sample_to_chunk_dump.push_back( e );
	}
	std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":stsc=" << sample_to_chunk_dump << std::endl;
#endif

	// stsz

	const auto& stsz = select<SampleSizeBox>(minf);
	assert( 1 == stsz.size() );

	// A stsz box may not have both sample_size and entry_sizes,
	// but a stsz box with neithwe of them is a valid empty stsz.
	assert( !( 0 != stsz[0]->data().sample_size && !stsz[0]->data().entry_sizes.empty() ) );

	std::vector<uint32_t> sample_sizes;

	if ( !stsz[0]->data().entry_sizes.empty() ) {
		sample_sizes.assign(
				stsz[0]->data().entry_sizes.begin() + ( ss.first - 1 ),
				stsz[0]->data().entry_sizes.begin() + ss.second );
	}
	else {
		sample_sizes.insert(
				sample_sizes.begin(), ss.second - ss.first + 1,
				stsz[0]->data().sample_size);
	}

#ifdef _DEBUG
	std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":stsz[" << sample_sizes.size() << "]=" << sample_sizes << std::endl;
#endif

	// stco

	const auto& stco = select<ChunkOffsetBox>(minf);
	if ( 1 == stco.size() && !stco[0]->data().chunk_offsets.empty() ) {
		std::vector<uint32_t> chunk_offsets;
		for ( auto ci = chunks.begin(); ci != chunks.end(); ci++ ) {
			assert( ss.first < (*ci).first_sample + (*ci).sample_count );
			assert( (*ci).first_sample <= ss.second );
			if ( (*ci).first_sample < ss.first ) {
				if ( !stsz[0]->data().entry_sizes.empty() ) {
					std::vector<uint32_t> samples(
							stsz[0]->data().entry_sizes.begin() + ( (*ci).first_sample - 1 ),
							stsz[0]->data().entry_sizes.begin() + ( ss.first - 1 ) );
					uint32_t sum = 0;
					for ( auto s: samples ) {
						sum += s;
					}
					chunk_offsets.push_back( (*ci).offset + sum );
				}
				else {
					assert( 0 != stsz[0]->data().sample_size );
					chunk_offsets.push_back( (*ci).offset + stsz[0]->data().sample_size * ( ss.first - (*ci).first_sample ) );
				}
			}
			else {
				chunk_offsets.push_back( (*ci).offset );
			}
		}

		stco[0]->data().chunk_offsets = chunk_offsets;

#ifdef _DEBUG
		std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":stco=" << stco[0]->data().chunk_offsets << std::endl;
#endif
	}

	// co64

	const auto& co64 = select<ChunkLargeOffsetBox>(minf);
	if ( 1 == co64.size() && !co64[0]->data().chunk_offsets.empty() ) {
		std::vector<uint64_t> chunk_offsets;
		for ( auto ci = chunks.begin(); ci != chunks.end(); ci++ ) {
			assert( ss.first < (*ci).first_sample + (*ci).sample_count );
			assert( (*ci).first_sample <= ss.second );
			if ( (*ci).first_sample < ss.first ) {
				if ( !stsz[0]->data().entry_sizes.empty() ) {
					std::vector<uint32_t> samples(
							stsz[0]->data().entry_sizes.begin() + ( (*ci).first_sample - 1 ),
							stsz[0]->data().entry_sizes.begin() + ( ss.first - 1 ) );
					uint32_t sum = 0;
					for ( auto s: samples ) {
						sum += s;
					}
					chunk_offsets.push_back( (*ci).offset + sum );
				}
				else {
					assert( 0 != stsz[0]->data().sample_size );
					chunk_offsets.push_back( (*ci).offset + stsz[0]->data().sample_size * ( ss.first - (*ci).first_sample ) );
				}
			}
			else {
				chunk_offsets.push_back( (*ci).offset );
			}
		}

		co64[0]->data().chunk_offsets = chunk_offsets;

#ifdef _DEBUG
		std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":co64=" << co64[0]->data().chunk_offsets << std::endl;
#endif
	}

	assert(
			( 1 == stco.size()
				&& !stco[0]->data().chunk_offsets.empty()
				&& co64.empty() )
			|| ( 1 == co64.size()
				&& !co64[0]->data().chunk_offsets.empty()
				&& stco.empty() )
	      );

	// stts

	const auto& stts = select<TimeToSampleBox>(minf);
	assert( 1 == stts.size() );
	assert( !stts[0]->data().entries.empty() );
	std::vector<TimeToSampleBox::Entry> stts_entries;
	uint32_t sample_count = 0;
	for ( auto e: stts[0]->data().entries ) {
		if ( ss.first <= sample_count + e.sample_count ) {
			uint32_t s1 = ( sample_count < ss.first - 1 ) ? ss.first - 1 : sample_count;
			uint32_t s2 = ( ss.second < sample_count + e.sample_count ) ? ss.second : sample_count + e.sample_count;
			//uint32_t upper = ( ss.second < sample_count + e.sample_count ) ? ss.second : sample_count + e.sample_count;
			//uint32_t lower = ( sample_count < ss.first ) ? ss.first : sample_count;
			uint32_t sc = s2 - s1;
			stts_entries.push_back( { 
					sc,
					e.sample_delta } );
			duration += sc * e.sample_delta;
		}

		sample_count += e.sample_count;

		if ( ss.second <= sample_count ) {
			break;
		}
	}
	stts[0]->data().entries = stts_entries;

#ifdef _DEBUG
	std::vector<std::vector<uint32_t>> sttsdump;
	uint32_t stts_sample_cnt = 0;
	for ( auto s: stts_entries ) {
		std::vector<uint32_t> e;
		e.push_back( s.sample_count );
		e.push_back( s.sample_delta );
		sttsdump.push_back( e );
		stts_sample_cnt += s.sample_count;
	}
	std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":stts=" << sttsdump << ", SUM(sample_count)=" << stts_sample_cnt << std::endl;
#endif

	// stss

	const auto& stss = select<SyncSampleBox>(minf);
	if ( 1 == stss.size() && !stss[0]->data().sample_numbers.empty() ) {
		std::vector<uint32_t> sync_samples;
		for ( auto s: stss[0]->data().sample_numbers ) {
			if ( ss.first <= s && s <= ss.second ) {
				sync_samples.push_back( s - ss.first + 1 );
			}
		}
		stss[0]->data().sample_numbers = sync_samples;

#ifdef _DEBUG
		std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":stss=" << stss[0]->data().sample_numbers << std::endl;
#endif
	}

	// ctts

	const auto& ctts = select<CompositionOffsetBox>(minf);
	if ( 1 == ctts.size() && !ctts[0]->data().entries.empty() ) {
		std::vector<CompositionOffsetBox::Entry> composition_offsets;
		uint32_t sample_cnt = 0;
		auto iter = ctts[0]->data().entries.begin();

		for ( ; iter != ctts[0]->data().entries.end(); iter++ ) {
			sample_cnt += (*iter).sample_count;
			if ( ss.first <= sample_cnt ) {
				// reached to the lower bound
				break;
			}
		}

		// reached to the lower bound of the segment
		// -> add the first ctts entry
		composition_offsets.push_back( {
				sample_cnt + 1 - ss.first,
				(*iter++).sample_offset } );

		//assert( ss.first < sample_cnt + (*iter).sample_count );

		for ( ; iter != ctts[0]->data().entries.end(); iter++ ) {
			sample_cnt += (*iter).sample_count;
			if ( ss.second <= sample_cnt ) {
				// reached to the upper bound of the segment
				// -> add the last ctts entry
				break;
			}

			composition_offsets.push_back( {
					(*iter).sample_count,
					(*iter).sample_offset } );
		}

		composition_offsets.push_back( {
				(*iter).sample_count - ( sample_cnt - ss.second ),
				(*iter).sample_offset } );

		ctts[0]->data().entries = composition_offsets;

#ifdef _DEBUG
		std::vector<std::vector<uint32_t>> cttsdump;
		for ( auto s: composition_offsets ) {
			std::vector<uint32_t> e;
			e.push_back( s.sample_count );
			e.push_back( s.sample_offset );
			cttsdump.push_back( e );
		}
		std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":ctts=" << cttsdump << std::endl;
#endif
	}

	stsz[0]->data().sample_size = 0;
	stsz[0]->data().entry_sizes = sample_sizes;
	stsc[0]->data().entries = sample_to_chunk_entries;

	return true;
}

bool MP4Segment::divide(
		std::shared_ptr<mp4_abstract_box> minf,
		uint32_t media_timescale,
		std::pair<uint64_t, uint64_t> segment_time_target_range,
		uint64_t& duration)
{
	assert( MINF == minf->head().boxtype );

	std::pair<uint64_t, uint64_t> mdia_segment_time_range;
	std::pair<uint32_t, uint32_t> mdia_segment_sample_range;

	if ( mdia_segment_time_and_sample_range(
				minf, media_timescale,
				segment_time_target_range,
				mdia_segment_time_range,
				mdia_segment_sample_range) ) {

		assert( 1 <= mdia_segment_sample_range.first );

		// There are cases that upper bound is smaller than the lowr bound (when segments are requested outside the file boundary)
		//assert( mdia_segment_sample_range.first <= mdia_segment_sample_range.second );
		//assert( mdia_segment_time_range.first <= mdia_segment_time_range.second );

		if ( 1 <= mdia_segment_sample_range.first
				&& mdia_segment_sample_range.first <= mdia_segment_sample_range.second
				&& mdia_segment_time_range.first <= mdia_segment_time_range.second ) {

			std::vector<Chunk> chunks;
			if ( sample_to_chunk(minf, mdia_segment_sample_range, chunks) ) {
				if ( minf_divide_samples_and_chunks(
							minf,
							mdia_segment_sample_range,
							chunks,
							duration) ) {
					return true;
				}
			}
		}
	}

	return false;
}

// create/update edts
void MP4Segment::update(
		std::shared_ptr<mp4_abstract_box> trak,
		uint32_t movie_timescale,
		uint32_t media_timescale,
		uint64_t media_time,
		uint64_t duration)
{
	assert( TRAK == trak->head().boxtype );

	// edts
	const auto& edts = select(trak, EDTS);
	if ( !edts.empty() ) {
		// elst
		const auto& elst = select<EditListBox>(trak);
		assert( elst.empty() || 1 == elst.size() );
		if ( !elst.empty() ) {

			EditListBox::Entry segment = {
				duration * movie_timescale / media_timescale,
				0, 1, 0 };

			for ( auto e: elst[0]->data().entries ) {
				// find the first element that has media_time != -1
				if ( -1 != e.media_time ) {
					segment.media_time = e.media_time;
					segment.media_rate_integer = e.media_rate_integer;
					segment.media_rate_fraction = e.media_rate_fraction;
					break;
				}
			}

			elst[0]->data().entries.clear();

			if ( 0 < media_time ) {
				elst[0]->data().entries.push_back( {
						media_time * movie_timescale / media_timescale,
						-1, 1, 0 } );
			}

			elst[0]->data().entries.push_back( segment );
			return;
		}

		// edts box exists but no elst?
		assert( false );
	}

	// no edts/elst is pre-existing

	std::shared_ptr<mp4_container_box> new_edts = std::make_shared<mp4_container_box>( BoxHead {
			0, 0, 0, EDTS, 1, 0
			} );
	std::shared_ptr<mp4_concrete_box<EditListBox>> new_elst = std::make_shared<mp4_concrete_box<EditListBox>>( BoxHead {
			0, 0, 0, ELST, 1, 0
			} );
	*trak << new_edts;
	*new_edts << new_elst;

	if ( 0 < media_time ) {
		new_elst->data().entries.push_back( {
				media_time * movie_timescale / media_timescale,
				-1, 1, 0 } );
	}

	new_elst->data().entries.push_back( {
			duration * movie_timescale / media_timescale,
			0, 1, 0 } );
}

// update mdat
void MP4Segment::update(std::shared_ptr<mp4_abstract_box> mp4)
{
	assert( MP4FILE == mp4->head().boxtype );

	const auto& mdat = select<MediaDataBox>(mp4);
	if ( mdat.empty() ) return;

	// mdat
	std::shared_ptr<mp4_concrete_box<MediaDataBox>> new_mdat = std::make_shared<mp4_concrete_box<MediaDataBox>>(
			BoxHead { 0, 0, 0, MDAT, 0, 0 }
			);

	for ( auto b: mdat ) {
		if ( new_mdat->data().uri.empty() ) {
			new_mdat->data().uri = b->data().uri;
		}
		else {
			assert( new_mdat->data().uri == b->data().uri );
		}

		// empty all existing mdat boxes.
		// a new mdat box will be created at the end of the file.
		b->remove();
		b->data().byte_ranges.clear();
		b->data().chunks.clear();
	}

	std::map<uint64_t, uint32_t> chunks;

	for ( auto trak: select(mp4, TRAK) ) {
		const auto& hdlr = select<HandlerBox>(trak);
		assert( 1 == hdlr.size() );

		// If no HDLR, skip.
		if ( hdlr.empty() ) continue;

		// If neither video nor audio, skip.
		if ( HandlerBox::VIDEO != hdlr[0]->data().handler_type
				&& HandlerBox::AUDIO != hdlr[0]->data().handler_type ) continue;

		// This track is either video or audio

		// stsc
		const auto& stsc = select<SampleToChunkBox>(trak);
		assert( 1 == stsc.size() );
		if ( stsc.empty() ) continue;

		std::map<uint32_t, uint32_t> chunk_to_sample_counts;
		uint32_t samples_per_chunk = 0;
		for ( auto sci = stsc[0]->data().entries.begin(); sci != stsc[0]->data().entries.end(); sci++ ) {
			chunk_to_sample_counts[(*sci).first_chunk] = samples_per_chunk;
			samples_per_chunk = (*sci).samples_per_chunk;
		}
		chunk_to_sample_counts[ 0xFFFFFFFF ] = samples_per_chunk;

#ifdef _DEBUG
		std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":chunk_to_sample_counts=" << chunk_to_sample_counts << std::endl;
#endif

		// stsz
		const auto& stsz = select<SampleSizeBox>(trak);
		assert( 1 == stsz.size() );
		if ( stsz.empty() ) continue;
		const auto& sample_sizes = stsz[0]->data().entry_sizes;
		auto szi = sample_sizes.begin();

		if ( szi != sample_sizes.end() ) {
			// stco, co64
			const auto& stco = select<ChunkOffsetBox>(trak);
			const auto& co64 = select<ChunkLargeOffsetBox>(trak);
			assert( ( 1 == stco.size() && co64.empty() ) || ( stco.empty() && 1 == co64.size() ) );
			if ( stco.empty() && co64.empty() ) continue;

			if ( !stco.empty() ) {
				uint32_t chunk_cnt = 0;
				for ( auto chunk_offset: stco[0]->data().chunk_offsets ) {
					auto csi = chunk_to_sample_counts.upper_bound(++chunk_cnt);
					assert( csi != chunk_to_sample_counts.end() );
					if ( csi == chunk_to_sample_counts.end() ) break;
					// csi->second is number of samples of the chunk

					uint32_t chunk_size = 0;
					for ( uint32_t i = 0; i < csi->second; i++ ) {
						chunk_size += *szi++;
					}
					chunks[chunk_offset] = chunk_size;
				}
			}
			else if ( !co64.empty() ) {
				uint32_t chunk_cnt = 0;
				for ( auto chunk_offset: co64[0]->data().chunk_offsets ) {
					auto csi = chunk_to_sample_counts.upper_bound(++chunk_cnt);
					assert( csi != chunk_to_sample_counts.end() );
					if ( csi == chunk_to_sample_counts.end() ) break;
					// csi->second is number of samples of the chunk

					uint64_t chunk_size = 0;
					for ( uint32_t i = 0; i < csi->second; i++ ) {
						chunk_size += *szi++;
					}
					chunks[chunk_offset] = chunk_size;
				}
			}
		}

		assert( sample_sizes.end() == szi );
	}

	// Merge adjacent chunks

	std::map<uint64_t, uint32_t> byte_ranges;
	uint64_t offset = 0;
	uint32_t size = 0;

	for ( auto chunk: chunks ) {
		// chunk.first is chunk offset
		// chunk.second is chunk size

		if ( offset + size == chunk.first ) {
			// The chunk is merged with the previous chunk to make a larger block.
			size += chunk.second;
		}
		else {
			// There is a gap between the previous and current chunks.
			// The previous block ends, and a new block starts.

			if ( 0 != size ) {
				byte_ranges[offset] = size;
			}

			offset = chunk.first;
			size = chunk.second;
		}
	}

	if ( 0 != offset ) {
		byte_ranges[offset] = size;
	}

	// create a new mdat box.
	// to make sure mdat is located at the end of the file.
	new_mdat->data().chunks = chunks;
	new_mdat->data().byte_ranges = byte_ranges;
	mp4->addChild(new_mdat);
}
