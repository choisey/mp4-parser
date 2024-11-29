/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_abstract_action.h"
#include "mp4.h"
#include <assert.h>

#ifdef _DEBUG

#include <algorithm>

#define DURATION_DIFF_THRESHOLD	0.1

#define UNEXPECTED(B1, B2) std::cerr << "[WARNING] " << __FILE__ << '(' << __LINE__ << ") Unexpected box: " \
		<< (char) ( B1.boxtype / 0x1000000 ) \
		<< (char) ( ( B1.boxtype / 0x10000 ) & 0xff ) \
		<< (char) ( ( B1.boxtype / 0x100 ) & 0xff ) \
		<< (char) ( B1.boxtype & 0xff ) << '/' \
		<< (char) ( B2.boxtype / 0x1000000 ) \
		<< (char) ( ( B2.boxtype / 0x10000 ) & 0xff ) \
		<< (char) ( ( B2.boxtype / 0x100 ) & 0xff ) \
		<< (char) ( B2.boxtype & 0xff ) \
		<< std::endl;

MP4AbstractAction::MP4ValidateVisitor::MP4ValidateVisitor()
{
}

MP4AbstractAction::MP4ValidateVisitor::~MP4ValidateVisitor()
{
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, std::vector<std::shared_ptr<mp4_abstract_box>>& boxes)
{
	for ( auto child: boxes ) {
		if ( _XX_ == child->head().boxtype 
				|| FREE == child->head().boxtype 
				|| SKIP == child->head().boxtype
				|| META == child->head().boxtype
				|| UUID == child->head().boxtype )
			continue;


		switch ( head.boxtype ) {
			case MP4FILE:
				switch ( child->head().boxtype ) {
					case FTYP:
					case MOOF:
					case MDAT:
					case ABST:
					case SIDX:
						break;

					case MOOV:
						{
							assert( _movies.empty() );
							Movie mv;
							_movies.push_back( mv );
							break;
						}

					default:
						UNEXPECTED(head, child->head());
						break;
				}
				break;

			case MOOV:
				switch ( child->head().boxtype ) {
					case TRAK:
						{
							Track tr;
							tr.track_type = 0;
							_tracks.push_back(tr);
							break;
						}

					case MVHD:
					case UDTA:
						break;

					default:
						UNEXPECTED(head, child->head());
						break;
				}
				break;

			case TRAK:
				switch ( child->head().boxtype ) {
					case TKHD:
					case EDTS:
					case MDIA:
					case TREF:
					case UDTA:
						break;

					default:
						UNEXPECTED(head, child->head());
						break;
				}
				break;

			case MDIA:
				switch ( child->head().boxtype ) {
					case MDHD:
					case HDLR:
					case MINF:
						break;

					default:
						UNEXPECTED(head, child->head());
						break;
				}
				break;

			case MINF:
				switch ( child->head().boxtype ) {
					case DINF:
					case STBL:
					case VMHD:
					case SMHD:
					case HMHD:
					case NMHD:
						break;

					default:
						UNEXPECTED(head, child->head());
						break;
				}
				break;

			case STBL:
				switch ( child->head().boxtype ) {
					case STSD:
					case STTS:
					case CTTS:
					case STSC:
					case STSZ:
					case STCO:
					case CO64:
					case STSS:
						break;

					default:
						UNEXPECTED(head, child->head());
						break;
				}
				break;

			case STSD:
				break;

			case MOOF:
				switch ( child->head().boxtype ) {
					case MFHD:
					case TRAF:
						break;

					default:
						UNEXPECTED(head, child->head());
						break;
				}
				break;

			case TRAF:
				switch ( child->head().boxtype ) {
					case TFHD:
					case TRUN:
					case TFDT:
						break;

					default:
						UNEXPECTED(head, child->head());
						break;
				}
				break;

			case EDTS:
				switch ( child->head().boxtype ) {
					case ELST:
						break;

					default:
						UNEXPECTED(head, child->head());
						break;
				}
				break;

			case DINF:
				switch ( child->head().boxtype ) {
					case DREF:
					case URL:
					case URN:
						break;

					default:
						UNEXPECTED(head, child->head());
						break;
				}
				break;

			case DREF:
				switch ( child->head().boxtype ) {
					case URL:
					case URN:
						break;

					default:
						UNEXPECTED(head, child->head());
						break;
				}
				break;

			case MVEX:
			case MFRA:
			case UDTA:
				break;

			default:
				std::cerr << "[WARNING] " << __FILE__ << '(' << __LINE__ << ") "
					<< " Unexpected box: "
					<< (char) ( head.boxtype / 0x1000000 )
					<< (char) ( ( head.boxtype / 0x10000 ) & 0xff )
					<< (char) ( ( head.boxtype / 0x100 ) & 0xff )
					<< (char) ( head.boxtype & 0xff )
					<< std::endl;
				break;
		}

		child->accept(this);
	}

	switch ( head.boxtype ) {
		case MP4FILE:
			{
				// #moov = 1
				if ( 1 != _movies.size() ) {
					std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") " << " #moov=" << _movies.size() << std::endl;
				}

				// chunks == mdat
				std::map<uint64_t, std::pair<uint64_t, uint64_t>> offset_to_chunk;
				for ( auto t = _tracks.begin(); t != _tracks.end(); t++ ) {
					if ( (*t).sample_to_chunks.empty() ) {
						std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") " << "stsc is empty" << std::endl;
					}

					if ( (*t).chunk_offsets.empty() ) {
						std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") " << "stco(co64) is empty" << std::endl;
					}

					if ( (*t).sample_sizes.empty() ) {
						std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") " << "stsz is empty" << std::endl;
					}

					if ( (*t).sample_to_chunks.empty() || (*t).chunk_offsets.empty() || (*t).sample_sizes.empty() ) {
						continue;
					}

					//assert( !(*t).sample_to_chunks.empty() );
					//assert( !(*t).chunk_offsets.empty() );
					//assert( !(*t).sample_sizes.empty() );

					// construct chunk list (specific to the track)

					std::vector<std::pair<uint64_t, uint64_t>> chunks_in_track;

					auto stsc_i = (*t).sample_to_chunks.begin();
					auto stco_i = (*t).chunk_offsets.begin();
					auto stsz_i = (*t).sample_sizes.begin();
					uint32_t chunk = 1;
					uint32_t samples_per_chunk = 1;

					for ( ; stsc_i != (*t).sample_to_chunks.end(); stsc_i++ ) {
						for ( ; chunk < (*stsc_i).first_chunk; chunk++ ) {
							uint32_t size = 0;
							for ( uint32_t i = 0; i < samples_per_chunk; i++ ) {
								assert( (*t).sample_sizes.end() != stsz_i );
								size += *stsz_i++;
							}

							assert( (*t).chunk_offsets.end() != stco_i );
							chunks_in_track.push_back( std::make_pair(*stco_i++, size) );
						}

						samples_per_chunk = (*stsc_i).samples_per_chunk;
					}

					for ( ; stco_i != (*t).chunk_offsets.end(); stco_i++ ) {
						uint32_t size = 0;
						for ( uint32_t i = 0; i < samples_per_chunk; i++ ) {
							assert( (*t).sample_sizes.end() != stsz_i );
							size += *stsz_i++;
						}

						chunks_in_track.push_back( std::make_pair(*stco_i, size) );
					}

					assert( stsc_i == (*t).sample_to_chunks.end() );
					assert( stco_i == (*t).chunk_offsets.end() );
					assert( stsz_i == (*t).sample_sizes.end() );

					// update chunk map (across tracks)

					for ( auto c = chunks_in_track.begin(); c != chunks_in_track.end(); c++ ) {
						assert( 0 != c->second );
						uint64_t offset = c->first;
						uint64_t size = c->second;

						auto prev_i = offset_to_chunk.find(offset);
						if ( prev_i != offset_to_chunk.end() ) {
							offset = prev_i->second.first;
							size += prev_i->second.second;
							offset_to_chunk.erase(prev_i);
						}

						auto next_i = offset_to_chunk.upper_bound(offset + size);
						if ( next_i != offset_to_chunk.end() ) {
							if ( offset + size == next_i->second.first ) {
								next_i->second.first = offset;
								next_i->second.second += size;
								continue;
							}
							else if ( next_i->second.first < offset + size ) {
								assert( false );
							}
						}

						offset_to_chunk[offset + size] = std::make_pair(offset, size);
					}
				}

				std::vector<std::pair<uint64_t, uint64_t>> merged_chunks;
				for ( auto iter = offset_to_chunk.begin(); iter != offset_to_chunk.end(); iter++ ) {
					merged_chunks.push_back( std::make_pair(iter->second.first, iter->second.second) );
				}

				if ( merged_chunks != _chunks ) {
					std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") " << "mdat" << _chunks << " != chunks" << merged_chunks << std::endl;
				}

				//stss
				for ( auto t = _tracks.begin(); t != _tracks.end(); t++ ) {
					switch ( (*t).track_type ) {
						case HandlerBox::VIDEO:
							if ( (*t).sync_samples.empty() ) {
								std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") " << "stss box missing in a video track" << std::endl;
							}
							break;

						default:
							if ( !(*t).sync_samples.empty() ) {
								std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") " << "stss box in a non-video track" << std::endl;
							}
					}
				}

				// stts & ctts
				for ( auto t = _tracks.begin(); t != _tracks.end(); t++ ) {

					// stts

					if ( (*t).time_to_samples.empty() ) {
						std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") " << "stts is empty" << std::endl;
					}

					std::vector<uint64_t> sample_times;
					uint32_t sample_time = 0;
					for ( auto st: (*t).time_to_samples ) {
						for ( uint32_t i = 0; i < st.sample_count; i++ ) {
							sample_times.push_back(sample_time);
							sample_time += st.sample_delta;
						}
					}

					if ( sample_times.size() != (*t).sample_sizes.size() ) {
						std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") "
							<< "SUM(stts.sample_count) " << sample_times.size() << " != COUNT(stsz.sample_size) " << (*t).sample_sizes.size() << std::endl;
					}

					if ( (*t).mdhd.empty() ) {
						std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") " << "stts has no mdhd" << std::endl;
						continue;
					}

					MediaHeaderBox& mdhd = (*t).mdhd.back();
					if ( mdhd.duration != sample_time ) {
						std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") " << "mdhd.duration "
							<< mdhd.duration << "(=" << ((double) mdhd.duration / mdhd.timescale) << "s) != SUM(stts.delta) "
							<< sample_time << "(=" << ((double) sample_time / mdhd.timescale) << "s)" << std::endl;
					}

					// ctts

					if ( HandlerBox::VIDEO != (*t).track_type ) {
						if ( !(*t).composition_offsets.empty() ) {
							std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") " << "ctts box in a non-video track" << std::endl;
						}
					}

					// ctts is optional
					if ( !(*t).composition_offsets.empty() ) {
						std::vector<uint64_t> composition_offsets;
						auto sti = sample_times.begin();
						for ( auto ct: (*t).composition_offsets ) {
							for ( uint32_t i = 0; i < ct.sample_count; i++ ) {
								composition_offsets.push_back(ct.sample_offset);
							}
						}

						// stts.sample_count == ctts.sample_count
						if ( sample_times.size() != composition_offsets.size() ) {
							std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") "
								<< "SUM(stts.sample_count) " << sample_times.size() << " != SUM(ctts.sample_count) " << composition_offsets.size() << std::endl;
						}

						// gop boundaries
						std::vector<std::pair<uint64_t, uint64_t>> gops;
						size_t si = 1;
						std::vector<uint32_t> sync_samples = (*t).sync_samples;
						sync_samples.push_back((*t).sample_sizes.size() + 1);
						for ( auto ss: sync_samples ) {
							std::vector<uint64_t> composition_times;
							for ( ; si < ss; si++ ) {
								if ( sample_times.size() <= si || composition_offsets.size() <= si ) {
									break;
								}
								composition_times.push_back(sample_times[si] + composition_offsets[si]);
							}

							if ( !composition_times.empty() ) {
								std::sort(composition_times.begin(), composition_times.end());
								gops.push_back(std::make_pair(composition_times.front(), composition_times.back()));
							}
						}

						uint64_t cur = 0;
						for ( auto gop: gops ) {
							if ( gop.first <= cur ) {
								std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") GOP boundary violation: " << gops << std::endl;
								break;
							}
							cur = gop.second;
						}
					}
				}

				break;
			}

		case TRAK:
			//assert( !_tracks.empty() );
			//validateTrack(_tracks.back());
			break;
	}
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, FileTypeBox& ftyp)
{
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, MovieHeaderBox& mvhd)
{
	assert( 1 == _movies.size() );
	assert( _movies.back().mvhd.empty() );
	_movies.back().mvhd.push_back(mvhd);
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, MovieExtendsHeaderBox& mehd)
{
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, TrackHeaderBox& tkhd)
{
	assert( 1 == _movies.size() );
	assert( 1 == _movies.back().mvhd.size() );
	const auto& mvhd = _movies.back().mvhd.back();

	assert( !_tracks.empty() );
	assert( _tracks.back().tkhd.empty() );
	_tracks.back().tkhd.push_back( tkhd );

	if ( (double) ( mvhd.duration - tkhd.duration ) / mvhd.timescale < -DURATION_DIFF_THRESHOLD
			|| DURATION_DIFF_THRESHOLD < (double) ( mvhd.duration - tkhd.duration ) / mvhd.timescale ) {
		std::cerr << "[WARNING] " << __FILE__ << '(' << __LINE__ << ") mvhd.duration "
			<< (double) mvhd.duration / mvhd.timescale << "s != tkhd.duration " << (double) tkhd.duration / mvhd.timescale << "s" << std::endl;
	}
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, MediaHeaderBox& mdhd)
{
	assert( 1 == _movies.size() );
	assert( 1 == _movies.back().mvhd.size() );
	const auto& mvhd = _movies.back().mvhd.back();

	assert( !_tracks.empty() );
	assert( _tracks.back().mdhd.empty() );
	_tracks.back().mdhd.push_back( mdhd );

	if ( ( (double) mdhd.duration / mdhd.timescale ) - ( (double) mvhd.duration / mvhd.timescale ) < -DURATION_DIFF_THRESHOLD
			|| DURATION_DIFF_THRESHOLD < ( (double) mdhd.duration / mdhd.timescale ) - ( (double) mvhd.duration / mvhd.timescale ) ) {
		std::cerr << "[ERROR] " << __FILE__ << '(' << __LINE__ << ") mvhd.duration "
			<< (double) mvhd.duration / mvhd.timescale << "s != mdhd.duration " << (double) mdhd.duration / mdhd.timescale << "s" << std::endl;
	}
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, HandlerBox& hdlr)
{
	assert( !_tracks.empty() );
	assert( 0 == _tracks.back().track_type );

	_tracks.back().track_type = hdlr.handler_type;
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, VideoMediaHeaderBox& vmhd)
{
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, SoundMediaHeaderBox& smhd)
{
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, HintMediaHeaderBox& hmhd)
{
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, TimeToSampleBox& stts)
{
	assert( !_tracks.empty() );
	auto& time_to_samples = _tracks.back().time_to_samples;
	assert( time_to_samples.empty() );
	time_to_samples.insert(time_to_samples.end(), stts.entries.begin(), stts.entries.end());
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, CompositionOffsetBox& ctts)
{
	assert( !_tracks.empty() );
	auto& composition_offsets = _tracks.back().composition_offsets;
	assert( composition_offsets.empty() );
	composition_offsets.insert(composition_offsets.end(), ctts.entries.begin(), ctts.entries.end());
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, SampleToChunkBox& stsc)
{
	assert( !_tracks.empty() );
	auto& sample_to_chunks = _tracks.back().sample_to_chunks;
	assert( sample_to_chunks.empty() );
	sample_to_chunks.insert(sample_to_chunks.end(), stsc.entries.begin(), stsc.entries.end());
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, SampleSizeBox& stsz)
{
	assert( !_tracks.empty() );
	auto& sample_sizes = _tracks.back().sample_sizes;
	assert( sample_sizes.empty() );
	sample_sizes.insert(sample_sizes.end(), stsz.entry_sizes.begin(), stsz.entry_sizes.end());
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, ChunkOffsetBox& stco)
{
	assert( !_tracks.empty() );
	auto& chunk_offsets = _tracks.back().chunk_offsets;
	assert( chunk_offsets.empty() );
	chunk_offsets.insert(chunk_offsets.end(), stco.chunk_offsets.begin(), stco.chunk_offsets.end());
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, ChunkLargeOffsetBox& co64)
{
	assert( !_tracks.empty() );
	auto& chunk_offsets = _tracks.back().chunk_offsets;
	assert( chunk_offsets.empty() );
	chunk_offsets.insert(chunk_offsets.end(), co64.chunk_offsets.begin(), co64.chunk_offsets.end());
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, SyncSampleBox& stss)
{
	assert( !_tracks.empty() );
	auto& sync_samples = _tracks.back().sync_samples;
	assert( sync_samples.empty() );
	sync_samples.insert(sync_samples.end(), stss.sample_numbers.begin(), stss.sample_numbers.end());
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, EditListBox& elst)
{
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, TrackFragmentHeaderBox& tfhd)
{
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, TrackFragmentDecodeTimeBox& tfdt)
{
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, TrackRunBox& trun)
{
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, SegmentIndexBox& sidx)
{
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, TrackExtendsBox& trex)
{
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, MediaDataBox& mdat)
{
	uint64_t offset = head.offset + head.boxheadsize;
	uint64_t size = head.boxsize - head.boxheadsize;
	_chunks.push_back( std::make_pair(offset, size) );
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, DataBox<std::vector<uint8_t>>& box)
{
}

void MP4AbstractAction::MP4ValidateVisitor::visit(BoxHead& head, EmptyBox& e)
{
}

#endif

MP4AbstractAction::MP4AbstractAction()
{
}

MP4AbstractAction::~MP4AbstractAction()
{
}

#ifdef _DEBUG

bool MP4AbstractAction::validate_moov(std::shared_ptr<mp4_abstract_box> moov, Context& ctx)
{
	bool valid = true;

/*
	// mvhd
	const auto& mvhd = select<MovieHeaderBox>(moov);
	if ( 1 == mvhd.size() ) {
		ctx.mvhd.timescale = mvhd[0]->data().timescale;
		ctx.mvhd.duration = mvhd[0]->data().duration;
		assert( 0 != ctx.mvhd.timescale );
	}
	else {
		ctx.mvhd.timescale = 1;
		ctx.mvhd.duration = 0;
		valid = false;
	}

	std::cerr << STATUS(valid)
		<< " #mvhd=" << mvhd.size()
		<< ", timescale=" << ctx.mvhd.timescale
		<< ", duration=" << ctx.mvhd.duration << "(=" << (double) ctx.mvhd.duration / ctx.mvhd.timescale << ')' << std::endl;
*/

	int track_cnt = 0;
	for ( auto trak: select(moov, TRAK) ) {
		if ( !validate_trak(trak, ctx) ) {
			valid = false;
		}
	}

	return valid;
}

bool MP4AbstractAction::validate_trak(std::shared_ptr<mp4_abstract_box> trak, Context& ctx)
{
	bool valid = true;

/*
	// tkhd
	const auto& tkhd = select<TrackHeaderBox>(trak);
	if ( 1 == tkhd.size() ) {
		std::cerr << "[OK] #tkhd=" << tkhd.size()
			<< ", id=" << tkhd[0]->data().track_ID
			<< ", duration=" << tkhd[0]->data().duration << "(=" << (double) tkhd[0]->data().duration / ctx.mvhd.timescale << ')' << std::endl;
	}
	else {
		std::cerr << "[ERROR] #tkhd=" << tkhd.size() << std::endl;
		valid = false;
	}
*/

	// mdia
	const auto& mdia = select(trak, MDIA);
	//std::cerr << STATUS(1 == mdia.size()) << " #mdia=" << mdia.size() << std::endl;
	if ( 1 == mdia.size() ) {
		validate_mdia(mdia[0], ctx);

		// edts
		const auto& edts = select(trak, EDTS);
		if ( 1 == edts.size() ) {
			validate_edts(mdia[0], edts[0], ctx);
		}
	}
	else {
		valid = false;
	}

	return valid;
}

bool MP4AbstractAction::validate_mdia(std::shared_ptr<mp4_abstract_box> mdia, Context& ctx)
{
	std::vector<uint64_t> chunks;
	std::vector<uint32_t> samples;
	std::map<uint32_t, std::pair<uint32_t,uint32_t>> sample_deltas; // from stts

	// mdhd
	auto mdhd = select<MediaHeaderBox>(mdia);
/*
	if ( 1 == mdhd.size() ) {
		fprintf(stderr, "[OK] #mdhd=1, timescale=%u, duration=%lu(=%f)\n",
				mdhd[0]->data().timescale,
				mdhd[0]->data().duration,
				(double) mdhd[0]->data().duration / mdhd[0]->data().timescale);
	}
	else {
		fprintf(stderr, "[ERROR] #mdhd=%lu\n", mdhd.size());
	}

	// hdlr
	auto hdlr = select<HandlerBox>(mdia);
	if ( !hdlr.empty() ) {
		fprintf(stderr, "[%s] #hdlr=1, type=%c%c%c%c, name=%s\n",
				1 == hdlr.size() ? "OK" : "ERROR",
				hdlr[0]->data().handler_type[0],
				hdlr[0]->data().handler_type[1],
				hdlr[0]->data().handler_type[2],
				hdlr[0]->data().handler_type[3],
				hdlr[0]->data().name.c_str());
	}
	else {
		fprintf(stderr, "[ERROR] #hdlr=%lu\n", hdlr.size());
	}

	// stco
	const auto& stco = select<ChunkOffsetBox>(mdia);
	if ( !stco.empty() ) {
		std::cerr << STATUS( 1 == stco.size() ) << " #stco=" << stco.size()
			<< ", #chunks[" << stco[0]->data().chunk_offsets.size() << "]=" << stco[0]->data().chunk_offsets << std::endl;
		chunks.insert(chunks.end(), stco[0]->data().chunk_offsets.begin(), stco[0]->data().chunk_offsets.end());
	}

	// co64
	const auto& co64 = select<ChunkLargeOffsetBox>(mdia);
	if ( !co64.empty() ) {
		std::cerr << STATUS( 1 == co64.size() ) << " #co64=" << co64.size()
			<< ", #chunks[" << co64[0]->data().chunk_offsets.size() << "]=" << co64[0]->data().chunk_offsets << std::endl;
		chunks.insert(chunks.end(), co64[0]->data().chunk_offsets.begin(), co64[0]->data().chunk_offsets.end());
	}

	if ( chunks.empty() ) {
		fprintf(stderr, "[ERROR] no chunks (stco or co64)\n");
	}

	// stsz
	const auto& stsz = select<SampleSizeBox>(mdia);
	if ( !stsz.empty() ) {
		samples = stsz[0]->data().entry_sizes;
		std::cerr << STATUS( 1 == stsz.size() ) << " #stsz=" << stsz.size()
			<< ", #samples[" << samples.size() << "]=" << samples << std::endl;
	}

	// stts
	auto stts = select<TimeToSampleBox>(mdia);
	if ( !stts.empty() ) {
		const auto& entries = stts[0]->data().entries;
		uint32_t total_samples = 0;
		uint32_t total_duration = 0;
		for ( auto e: entries ) {
			total_samples += e.sample_count;
			total_duration += e.sample_count * e.sample_delta;

			sample_deltas[total_samples] = std::make_pair(e.sample_count, e.sample_delta);
		}
		fprintf(stderr, "[%s] #stts=%lu, #deltas=%lu, (+sample_count=last_sample, delta)={ ",
				!stts.empty() ? "OK" : "ERROR", stts.size(), entries.size());
		int count = 0;
		for ( auto delta: sample_deltas ) {
			if ( MAX_ENTRIES_TO_LIST <= count++ ) {
				fprintf(stderr, "... ");
				break;
			}
			fprintf(stderr, "(+%u=%u, %u), ", delta.second.first, delta.first, delta.second.second);
		}
		fprintf(stderr, "}, #samples=%u, total_duration=%f\n",
				total_samples,
				!mdhd.empty() ? (double) total_duration / mdhd[0]->data().timescale: 0);
	}
	else {
		fprintf(stderr, "[ERROR] No stts!\n");
	}

	// stsc
	std::map<uint32_t, uint32_t> first_samples;
	auto stsc = select<SampleToChunkBox>(mdia);
	if ( !stsc.empty() ) {
		const auto& entries = stsc[0]->data().entries;
		uint32_t chunk_i = 0;
		uint32_t sample_i = 0;
		uint32_t first_sample_in_chunk = 0;

		if ( !entries.empty() ) {
			auto iter = entries.begin();
			chunk_i = 1;
			uint32_t samples_per_chunk = (*iter).samples_per_chunk;
			iter++;

			first_samples[1] = 0;
			first_sample_in_chunk = 1;

			for ( ; iter != entries.end(); iter++ ) {
				for ( ; chunk_i < (*iter).first_chunk; chunk_i++ ) {
					sample_i += samples_per_chunk;
					first_samples[sample_i + 1] = first_sample_in_chunk;
					first_sample_in_chunk = sample_i + 1;
				}
				samples_per_chunk = (*iter).samples_per_chunk;
			}
			while ( chunk_i <= chunks.size() ) {
				sample_i += samples_per_chunk;
				if ( chunks.size() <= chunk_i ) {
					break;
				}
				first_samples[sample_i + 1] = first_sample_in_chunk;
				first_sample_in_chunk = sample_i + 1;
				chunk_i++;
			}
			first_samples[sample_i + 1] = first_sample_in_chunk;
		}
		fprintf(stderr, "[%s] #stsc=%lu, #entries=%lu, #chunk=%u, #samples=%u, (first_sample, previous_first_sample)={ ",
				( 1 == stsc.size() && samples.size() == sample_i ) ? "OK" : "ERROR",
				stsc.size(), entries.size(), chunk_i, sample_i);
		int count = 0;
		for ( auto c: first_samples ) {
			if ( MAX_ENTRIES_TO_LIST <= count++ ) {
				fprintf(stderr, "... ");
				break;
			}
			fprintf(stderr, "(%u, %u), ", c.first, c.second);
		}
		fprintf(stderr, "}\n");
	}
	else {
		fprintf(stderr, "[ERROR] No stcs!\n");
	}

	// stss
	std::map<uint32_t, uint32_t> random_access_points;
	auto stss = select<SyncSampleBox>(mdia);
	if ( !stss.empty() ) {
		const auto& sample_numbers = stss[0]->data().sample_numbers;
		uint32_t random_access_point = 0;
		for ( auto s: sample_numbers ) {
			random_access_points[s] = random_access_point ? random_access_point : s;
			random_access_point = s;
		}
		random_access_points[ samples.size() ] = random_access_point;

		if ( !mdhd.empty() ) {
			fprintf(stderr, "[OK] #stss=%lu, #key_frames=%lu, (key_frame, key_frame_time)={ ", stss.size(), sample_numbers.size());
			int count = 0;
			for ( auto s: sample_numbers) {
				uint32_t delta_beginning = 0;
				uint64_t time = 0;
				if ( MAX_ENTRIES_TO_LIST <= count++ ) {
					fprintf(stderr, "... ");
					break;
				}

				for ( auto delta: sample_deltas ) {
					const auto& d = delta.second;
					if ( s <= d.first ) {
						time += (uint64_t) ( s - delta_beginning - 1 ) * d.second * 1000 / mdhd[0]->data().timescale;
						break;
					}
					delta_beginning = d.first;
					time += (uint64_t) d.first * d.second * 1000 / mdhd[0]->data().timescale;
				}
				fprintf(stderr, "(%u, %0.3f), ", s, (double) time / 1000);
			}
			fprintf(stderr, "}\n");
		}
		else {
			fprintf(stderr, "[ERROR] #stss=%lu, unable to process because mdhd is missing.\n", sample_numbers.size());
		}

		//				int err_count = 0;
		//				for ( auto s: sample_numbers) {
		//					if ( first_samples.find(s) == first_samples.end() ) {
		//						if ( MAX_OCCURRANCES_OF_ERROR <= err_count++ ) {
		//							fprintf(stderr, "[WARNING] ...\n");
		//							break;
		//						}
		//						fprintf(stderr, "[WARNING] sample[%u] is a random-access-point, but not the first sample ", s);
		//						auto sample_i = first_samples.lower_bound(s);
		//						if ( sample_i != first_samples.end() ) {
		//							fprintf(stderr, "in chunk[s#%u-s#%u] { ", sample_i->second, sample_i->first - 1);
		//							for ( size_t i = sample_i->second; i <= sample_i->first - 1; i++ ) {
		//								fprintf(stderr,
		//										( i == s ) ? ">>> %u <<<, " : "%u, ",
		//										samples[i-1]);
		//							}
		//						}
		//						fprintf(stderr, "}\n");
		//					}
		//				}
	}

	// ctts
	for ( auto ctts : select<CompositionOffsetBox>(mdia) ) {
		uint32_t sample_cnt = 0;
		for ( auto e: ctts->data().entries ) {
			sample_cnt += e.sample_count;
		}

		if ( sample_cnt != samples.size() ) {
			fprintf(stderr, "[ERROR] ctts: #samples=%u doesn't match with stsz.\n", sample_cnt);
		}

		std::map<uint64_t, uint32_t> composition_time_map;
		uint64_t composition_time = 0;
		uint32_t sample_i = 0;
		for ( auto e: ctts->data().entries ) {
			for ( uint32_t i = 0; i < e.sample_count; i++ ) {
				auto delta_i = sample_deltas.lower_bound(sample_i++);
				if ( delta_i != sample_deltas.end() ) {
					composition_time_map[ composition_time + e.sample_offset ] = sample_i;
					composition_time += delta_i->second.second;
				}
			}
		}

		uint32_t last_rap = 0;
		for ( auto i: composition_time_map) {
			auto rap_i = random_access_points.upper_bound(i.second);
			if ( rap_i != random_access_points.end() ) {
				//fprintf(stderr, "%lu:%u:%u, ", i.first, i.second, rap_i->second);
				if ( rap_i->second < last_rap ) {
					fprintf(stderr, "[ERROR] ctts: composition time near sample[%u] jumps across random access point\n", i.second);
				}
				last_rap = rap_i->second;
			}


		}
		fprintf(stderr, "ctts: #=%lu, SUM(sample_count)=%u\n", ctts->data().entries.size(), sample_cnt);
	}
*/

	// TODO:
	// ctts has to be scanned and checked to see if there is any sample that has decoding time crossing the GOP boundary.

	return true;
}

bool MP4AbstractAction::validate_edts(std::shared_ptr<mp4_abstract_box> mdia, std::shared_ptr<mp4_abstract_box> edts, Context& ctx)
{
	const auto& elst = select<EditListBox>(edts);
	assert ( elst.size() <= 1 );
	if ( !elst.empty() ) {
		const auto& entries = elst[0]->data().entries;
		//fprintf(stderr, "[%s] #elst=%lu, ",
		//		1 == edts.size() && 1 == elst.size() ? "OK" : "ERROR",
		//		edts.size());
		for ( auto e: entries ) {
			if ( -1 == e.media_time ) {
				assert( 0 != ctx.mvhd.timescale );
				fprintf(stderr, "{ empty, duration: %0.3f }, ", (double) e.segment_duration / ctx.mvhd.timescale);
			}
			else {
				auto mdhd = select<MediaHeaderBox>(mdia);
				if ( !mdhd.empty() ) {
					assert( 0 != ctx.mvhd.timescale );
					fprintf(stderr, "{ start: %ld(=%0.3f), duration: %lu(=%0.3f) }, ",
							e.media_time, (double) e.media_time / ctx.mvhd.timescale,
							e.segment_duration, (double) e.segment_duration / ctx.mvhd.timescale);
					continue;
				}

				fprintf(stderr, "[ERROR] media_time in edts/elst cannot be calculated without mdia/mdhd\n");
			}
		}
		fprintf(stderr, "\n");
	}

	return true;
}

bool MP4AbstractAction::validate(std::shared_ptr<mp4_abstract_box> box, Context& ctx)
{
	assert( MP4FILE ==  box->head().boxtype );

	// moov & mdat
	const auto& moov = select(box, MOOV);
	const auto& mdat = select(box, MDAT);

	for ( auto mb: mdat ) {
		ctx.mdat.push_back( std::pair<uint64_t, uint64_t>(
					mb->head().offset + mb->head().boxheadsize,
					mb->head().boxsize - mb->head().boxheadsize) );
	}

/*
	std::cerr << STATUS( 1 == moov.size() && 1 == mdat.size() )
		<< " #moov=" << moov.size() << ": box=" << moov
		<< ", #mdat=" << mdat.size() << ": box=" << mdat << ", data=" << ctx.mdat << std::endl;
*/

	bool valid = true;

	if ( !validate_moov(moov[0], ctx) ) {
		valid = false;
	}

	return valid;
}

#endif

void MP4AbstractAction::execute(std::shared_ptr<mp4_abstract_box> box)
{
#ifdef _DEBUG
	MP4ValidateVisitor v;
	box->accept(&v);
#endif
}

void MP4AbstractAction::execute(std::vector<std::shared_ptr<MP4File>>& mp4files)
{
	for ( auto mp4: mp4files ) {
		execute(mp4);
	}
}
