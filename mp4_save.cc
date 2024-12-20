/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_save.h"
#include "mp4_relocate.h"
//#include "mp4_file.h"
#include "mp4.h"
#include "io.h"
#include <assert.h>
#include <map>

// mp4_save_visitor

mp4_save::mp4_save_visitor::mp4_save_visitor(FILE* fp)
	: _fp(fp)
{
	assert( NULL != fp );
}

mp4_save::mp4_save_visitor::~mp4_save_visitor()
{
}

void mp4_save::mp4_save_visitor::write8b(uint8_t u8)
{
	fwrite(&u8, sizeof(uint8_t), 1, _fp);
}

void mp4_save::mp4_save_visitor::write16b(uint16_t u16)
{
	unsigned char buf[2];
	buf[0] = ( u16 / 0x100UL ) & 0xff;
	buf[1] = u16 & 0xff;
	fwrite(buf, sizeof(buf), 1, _fp);
}

void mp4_save::mp4_save_visitor::write32b(uint32_t u32)
{
	unsigned char buf[4];
	buf[0] = ( u32 / 0x1000000UL ) & 0xff;
	buf[1] = ( u32 / 0x10000UL ) & 0xff;
	buf[2] = ( u32 / 0x100UL ) & 0xff;
	buf[3] = u32 & 0xff;
	fwrite(buf, sizeof(buf), 1, _fp);
}

void mp4_save::mp4_save_visitor::write64b(uint64_t u64)
{
	unsigned char buf[8];
	buf[0] = u64 / 0x100000000000000UL;
	buf[1] = ( u64 / 0x1000000000000UL ) & 0xff;
	buf[2] = ( u64 / 0x10000000000UL ) & 0xff;
	buf[3] = ( u64 / 0x100000000UL ) & 0xff;
	buf[4] = ( u64 / 0x1000000UL ) & 0xff;
	buf[5] = ( u64 / 0x10000UL ) & 0xff;
	buf[6] = ( u64 / 0x100UL ) & 0xff;
	buf[7] = u64 & 0xff;
	fwrite(buf, sizeof(buf), 1, _fp);
}

void mp4_save::mp4_save_visitor::write4c(const char* cp)
{
	char buf[4] = { '\0', };
	for (int i = 0; i < 4; i++) {
		buf[i] = *cp++;
		if ( '\0' == buf[i] ) {
			break;
		}
	}
	fwrite(buf, sizeof(buf), 1, _fp);
}

void mp4_save::mp4_save_visitor::write_string(const std::string& str)
{
	fwrite(str.c_str(), str.size(), 1, _fp);
}

void mp4_save::mp4_save_visitor::write_box_head(const BoxHead& head)
{
//	if ( BOXSIZE_THRESHOLD < head.boxsize ) {
//		write32b(0x00000001);
//		write32b(head.boxtype);
//		write64b(head.boxsize);
//	}
//	else {
		write32b(head.boxsize);
		write32b(head.boxtype);
//	}

	if ( head.is_fullbox() ) {
		write32b(head.version * 0x1000000UL + head.flag);
	}

#ifdef _DEBUG
	fprintf(stderr, "** %s:%s(%d) head.boxtype=%c%c%c%c, head.offset=%lu, head.boxheadsize=%lu, head.boxsize=%lu\n", __func__, __FILE__, __LINE__,
			( head.boxtype & 0xff000000 ) / 0x1000000, ( head.boxtype & 0xff0000 ) / 0x10000, ( head.boxtype & 0xff00 ) / 0x100, head.boxtype & 0xff,
			head.offset, head.boxheadsize, head.boxsize);
#endif
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, std::vector<std::shared_ptr<mp4_abstract_box>>& boxes)
{
	assert( _XX_ != head.boxtype );

	if ( MP4FILE != head.boxtype ) {
		write_box_head(head);
	}

	// dref and stsd have different structure
	switch ( head.boxtype ) {
		case DREF:
		case STSD:
			write32b(boxes.size());
	}

	for ( auto child: boxes )
	{
		if ( _XX_ != child->head().boxtype ) {
			child->accept(this);
		}
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, FileTypeBox& ftyp)
{
	write_box_head(head);
	write4c(ftyp.major_brand.c_str());
	write4c(ftyp.minor_version.c_str());
	for ( auto brand: ftyp.compatible_brands ) {
		write4c(brand.c_str());
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, MovieHeaderBox& mvhd)
{
	write_box_head(head);

	switch ( head.version ) {
		case 1:
			write64b(mvhd.creation_time);
			write64b(mvhd.modification_time);
			write32b(mvhd.timescale);
			write64b(mvhd.duration);
			break;

		default: // head.version == 0
			write32b((uint32_t) mvhd.creation_time);
			write32b((uint32_t) mvhd.modification_time);
			write32b(mvhd.timescale);
			write32b((uint32_t) mvhd.duration);
			break;
	}

	write32b(0x00010000);
	write16b(0x0100);

	// reserved
	write16b(0);
	write32b(0); write32b(0);

	// matrix
	write32b(0x00010000); write32b(0); write32b(0);
	write32b(0); write32b(0x00010000); write32b(0);
	write32b(0); write32b(0); write32b(0x40000000);

	// pre-defined
	write32b(0); write32b(0); write32b(0);
	write32b(0); write32b(0); write32b(0);

	write32b(mvhd.next_track_ID);
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, MovieExtendsHeaderBox& mehd)
{
	write_box_head(head);
	switch ( head.version ) {
	case 1:
		write64b(mehd.fragment_duration);
		break;

	default:
		write32b((uint32_t) mehd.fragment_duration);
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, TrackHeaderBox& tkhd)
{
	write_box_head(head);

	switch ( head.version ) {
		case 1:
			write64b(tkhd.creation_time);
			write64b(tkhd.modification_time);
			write32b(tkhd.track_ID);
			write32b(0);
			write64b(tkhd.duration);
			break;

		default:
			write32b((uint32_t) tkhd.creation_time);
			write32b((uint32_t) tkhd.modification_time);
			write32b(tkhd.track_ID);
			write32b(0);
			write32b((uint32_t) tkhd.duration);
			break;
	}

	// reserved
	write32b(0); write32b(0);

	write16b(0);
	write16b(0);
	write16b(tkhd.volume);
	write16b(0);

	// matrix
	write32b(0x00010000); write32b(0); write32b(0);
	write32b(0); write32b(0x00010000); write32b(0);
	write32b(0); write32b(0); write32b(0x40000000);

	write32b(tkhd.width);
	write32b(tkhd.height);
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, MediaHeaderBox& mdhd)
{
	write_box_head(head);

	switch ( head.version ) {
		case 1:
			write64b(mdhd.creation_time);
			write64b(mdhd.modification_time);
			write32b(mdhd.timescale);
			write64b(mdhd.duration);
			break;

		default:
			write32b((uint32_t) mdhd.creation_time);
			write32b((uint32_t) mdhd.modification_time);
			write32b(mdhd.timescale);
			write32b((uint32_t) mdhd.duration);
			break;
	}

	write16b( ( ( mdhd.language[0] - 0x60 ) << 10 )
		+ ( ( mdhd.language[1] - 0x60 ) << 5 )
		+ ( mdhd.language[2] - 0x60 ));
	write16b(0);
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, HandlerBox& hdlr)
{
	write_box_head(head);
	write32b(0);
	write32b(hdlr.handler_type);

	// reserved
	write32b(0);
	write32b(0);
	write32b(0);

	write_string(hdlr.name);
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, VideoMediaHeaderBox& vmhd)
{
	write_box_head(head);
	write16b(vmhd.graphicsmode);
	write16b(vmhd.opcolor[0]);
	write16b(vmhd.opcolor[1]);
	write16b(vmhd.opcolor[2]);
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, SoundMediaHeaderBox& smhd)
{
	write_box_head(head);
	write16b(smhd.balance);
	write16b(0);
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, HintMediaHeaderBox& hmhd)
{
	write_box_head(head);
	write16b(hmhd.maxPDUsize);
	write16b(hmhd.avgPDUsize);
	write32b(hmhd.maxbitrate);
	write32b(hmhd.avgbitrate);
	write32b(0);
}

//void mp4_save::mp4_save_visitor::visit(BoxHead& head, SampleDescriptionBox::VisualSampleEntry& sd)
//{
//	write_box_head(head);
//	write32b(0);
//	write32b((uint32_t) sd.data_reference_index);
//
//	write16b(sd.pre_defined1);
//	write16b(sd.reserved1);
//	write32b(sd.pre_defined2[0]);
//	write32b(sd.pre_defined2[1]);
//	write32b(sd.pre_defined2[2]);
//	write16b(sd.width);
//	write16b(sd.height);
//	write32b(sd.horizresolution);
//	write32b(sd.vertresolution);
//	write32b(sd.reserved2);
//	write16b(sd.frame_count);
//	for ( auto c: sd.compressorname ) {
//		write8b(c);
//	}
//	write16b(sd.depth);
//	write16b(sd.pre_defined3);
//
//	for ( const auto& ext: sd.extensions ) {
//		write32b( ext->first.boxsize );
//		write32b( ext->first.boxtype );
//		for ( auto byte: ext->second ) {
//			write8b(byte);
//		}
//	}
//}
//
//void mp4_save::mp4_save_visitor::visit(BoxHead& head, SampleDescriptionBox::AudioSampleEntry& sd)
//{
//	write_box_head(head);
//	write32b(0);
//	write32b((uint32_t) sd.data_reference_index);
//
//	write32b(sd.reserved1[0]);
//	write32b(sd.reserved1[1]);
//	write16b(sd.channelcount);
//	write16b(sd.samplesize);
//	write16b(sd.pre_defined1);
//	write16b(sd.reserved2);
//	write32b(sd.samplerate);
//
//	for ( const auto& ext: sd.extensions ) {
//		write32b( ext->first.boxsize );
//		write32b( ext->first.boxtype );
//		for ( auto byte: ext->second ) {
//			write8b(byte);
//		}
//	}
//}
//
//void mp4_save::mp4_save_visitor::visit(BoxHead& head, SampleDescriptionBox::HintSampleEntry& sd)
//{
//	write_box_head(head);
//	write32b(0);
//	write32b((uint32_t) sd.data_reference_index);
//	for ( auto byte: sd.data ) {
//		write8b(byte);
//	}
//}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, TimeToSampleBox& stts)
{
	write_box_head(head);
	write32b(stts.entries.size());
	for ( auto e: stts.entries ) {
		write32b(e.sample_count);
		write32b(e.sample_delta);
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, CompositionOffsetBox& ctts)
{
	write_box_head(head);
	write32b(ctts.entries.size());
	for ( auto e: ctts.entries ) {
		write32b(e.sample_count);
		write32b(e.sample_offset);
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, SampleToChunkBox& stsc)
{
	write_box_head(head);
	write32b(stsc.entries.size());
	for ( auto e: stsc.entries ) {
		write32b(e.first_chunk);
		write32b(e.samples_per_chunk);
		write32b(e.sample_description_index);
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, SampleSizeBox& stsz)
{
	write_box_head(head);
	write32b(0);
	write32b(stsz.entry_sizes.size());
	for ( auto size: stsz.entry_sizes ) {
		write32b(size);
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, ChunkOffsetBox& stco)
{
	write_box_head(head);
	write32b(stco.chunk_offsets.size());
	for ( auto co: stco.chunk_offsets ) {
		write32b(co);
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, ChunkLargeOffsetBox& co64)
{
	write_box_head(head);
	write32b(co64.chunk_offsets.size());
	for ( auto co: co64.chunk_offsets ) {
		write64b(co);
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, SyncSampleBox& stss)
{
	write_box_head(head);
	write32b(stss.sample_numbers.size());
	for ( auto s: stss.sample_numbers ) {
		write32b(s);
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, SampleDependencyTypeBox& sdtp)
{
	// do nothing.
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, EditListBox& elst)
{
	write_box_head(head);
	write32b(elst.entries.size());
	for ( auto e: elst.entries ) {

		switch ( head.version ) {
			case 1:
				write64b(e.segment_duration);
				write64b((uint64_t) e.media_time);
				break;

			default: // version == 0
				write32b((uint32_t) e.segment_duration);
				write32b((uint32_t) e.media_time);
				break;
		}

		write16b((uint16_t) e.media_rate_integer);
		write16b((uint16_t) e.media_rate_fraction);
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, TrackFragmentHeaderBox& tfhd)
{
	write_box_head(head);
	write32b(tfhd.track_ID);
	if ( head.flag & 0x000001 ) write64b(tfhd.base_data_offset);
	if ( head.flag & 0x000002 ) write32b(tfhd.sample_description_index);
	if ( head.flag & 0x000008 ) write32b(tfhd.default_sample_duration);
	if ( head.flag & 0x000010 ) write32b(tfhd.default_sample_size);
	if ( head.flag & 0x000020 ) write32b(tfhd.default_sample_flags);
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, TrackFragmentDecodeTimeBox& tfdt)
{
	write_box_head(head);
	write64b(tfdt.decode_time);
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, TrackRunBox& trun)
{
	write_box_head(head);
	write32b(trun.samples.size());
	if ( head.flag & 0x000001 ) write32b(trun.data_offset);
	if ( head.flag & 0x000004 ) write32b(trun.first_sample_flags);
	for ( auto s: trun.samples ) {
		if ( head.flag & 0x000100 ) write32b(s.sample_duration);
		if ( head.flag & 0x000200 ) write32b(s.sample_size);
		if ( head.flag & 0x000400 ) write32b(s.sample_flags);
		if ( head.flag & 0x000800 ) write32b(s.sample_composition_time_offset);
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, SegmentIndexBox& sidx)
{
	write_box_head(head);
	if ( 1 == head.version ) {
		write32b(sidx.reference_id);
		write32b(sidx.timescale);
		write64b(sidx.earliest_presentation_time);
		write64b(sidx.first_offset);
		write16b((uint16_t) 0);
		write16b(sidx.references.size());
	}
	else { // version==0
		write32b(sidx.reference_id);
		write32b(sidx.timescale);
		write32b(sidx.earliest_presentation_time);
		write32b(sidx.first_offset);
		write16b((uint16_t) 0);
		write16b(sidx.references.size());
	}

	for ( auto r: sidx.references ) {
		uint32_t reference[3] = {
			(uint32_t) ( r.reference_type ? 0x80000000 : 0 ) | ( r.reference_size & 0x7FFFFFFF ),
			r.subsegment_duration,
			(uint32_t) ( r.contains_SAP ? 0x80000000 : 0 ) | ( r.SAP_delta_time & 0x7FFFFFFF )
		};

		write32b(reference[0]);
		write32b(reference[1]);
		write32b(reference[2]);
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, MovieFragmentHeaderBox& mfhd)
{
	write_box_head(head);
	write32b(mfhd.sequence_number);
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, TrackExtendsBox& trex)
{
	write_box_head(head);
	write32b(trex.track_ID);
	write32b(trex.default_sample_description_index);
	write32b(trex.default_sample_duration);
	write32b(trex.default_sample_size);
	write32b(trex.default_sample_flags);
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, MediaDataBox& mdat)
{
	if ( !mdat.byte_ranges.empty() ) {
		write_box_head(head);

		auto f = io::instance()->open(mdat.uri.c_str());
		if ( f->is_open() ) {
			//uint8_t buf[BUFFER_SIZE];
			//auto buf = f->allocate_block();
                        auto buf = std::make_unique<block>( BUFSIZ );

#ifdef _DEBUG
			uint32_t debug_byte_ranges = 0;
			uint64_t debug_bytes = 0;
			fprintf(stderr, "** %s:%s(%d) mdat.byte_ranges[%lu]={ ", __func__, __FILE__, __LINE__, mdat.byte_ranges.size());
			int blockcnt = 0;
			for ( auto b: mdat.byte_ranges ) {
				if ( blockcnt++ ) fprintf(stderr, ", ");
				fprintf(stderr, "%lu-%lu/%u[", b.first, b.first + b.second - 1, b.second);
				debug_byte_ranges++;
				debug_bytes += b.second;

				f->seek(b.first, SEEK_SET);
				size_t bytes_to_copy = b.second;
				int bufcnt = 0;
				while ( 0 < bytes_to_copy ) {
					size_t n;

					if ( buf->size() < bytes_to_copy ) {
						n = buf->size();
						bytes_to_copy -= buf->size();
					}
					else {
						n = bytes_to_copy;
						bytes_to_copy = 0;
					}

					if ( 0 < f->read(*buf, n) ) {
						if ( bufcnt++ ) fprintf(stderr, ", ");
						fprintf(stderr, "%lu", n);
					}
				}
				fprintf(stderr, "]");
			}
			fprintf(stderr, " }, #block=%u, #bytes=%lu\n", debug_byte_ranges, debug_bytes);
#endif

			for ( auto b: mdat.byte_ranges ) {
				f->seek(b.first, SEEK_SET);
				size_t bytes_to_copy = b.second;
				while ( 0 < bytes_to_copy ) {
					size_t n;

					if ( buf->size() < bytes_to_copy ) {
						n = buf->size();
						bytes_to_copy -= buf->size();
					}
					else {
						n = bytes_to_copy;
						bytes_to_copy = 0;
					}

					if ( 0 < f->read(*buf, n) ) {
						fwrite(*buf, n, 1, _fp);
					}
				}
			}

			f->close();
		}
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, DataBox<std::vector<uint8_t>>& box)
{
	if ( 0 != head.boxsize ) {
		write_box_head(head);
		for ( auto b: box.boxdata ) {
			write8b(b);
		}
	}
}

void mp4_save::mp4_save_visitor::visit(BoxHead& head, EmptyBox& e)
{
	// do nothing.
}

// mp4_save

mp4_save::mp4_save()
{
}

mp4_save::mp4_save(const char* filename)
	: _filename(filename)
{
}

mp4_save::~mp4_save()
{
}

void mp4_save::execute(std::shared_ptr<mp4_abstract_box> mp4)
{
	assert( typeid(*mp4) == typeid(mp4_file) );
	assert( MP4FILE == mp4->head().boxtype );

	// Recalculate box offsets
	Relocate(mp4);

	// Build offset mapping table (old offset -> new offset)
	std::map<uint64_t, uint64_t> chunk_relocation_map;

	for ( const auto& mdat: select<MediaDataBox>(mp4) ) {
		uint64_t pos = mdat->head().offset + mdat->head().boxheadsize;
		for ( auto chunk: mdat->data().chunks ) {
			chunk_relocation_map[chunk.first] = pos;
			pos += chunk.second;
		}
	}

#ifdef _DEBUG
	fprintf(stderr, "** %s:%s(%d) chunk_relocation_map={ ", __func__, __FILE__, __LINE__);
	for ( auto cc: chunk_relocation_map ) {
		fprintf(stderr, "%lu -> %lu, ", cc.first, cc.second);
	}
	fprintf(stderr, "}\n");
#endif

	// stco
	for ( auto stco: select<ChunkOffsetBox>(mp4) ) {
		for ( auto& chunk_offset: stco->data().chunk_offsets ) {
			auto iter = chunk_relocation_map.find(chunk_offset);
			//assert( iter != chunk_relocation_map.end() );
			if ( iter != chunk_relocation_map.end() ) {
				chunk_offset = iter->second;
			}
		}
	}

	// co64
	for ( auto co64: select<ChunkLargeOffsetBox>(mp4) ) {
		for ( auto& chunk_offset: co64->data().chunk_offsets ) {
			auto iter = chunk_relocation_map.find(chunk_offset);
			assert( iter != chunk_relocation_map.end() );
			if ( iter != chunk_relocation_map.end() ) {
				chunk_offset = iter->second;
			}
		}
	}

	if ( !_filename.empty() ) {
		// Write to file
		FILE* fp = fopen(_filename.c_str(), "w");
		assert( NULL != fp );
		if ( NULL != fp ) {
			mp4_save_visitor v(fp);
			mp4->accept(&v);
			fclose(fp);
		}
	}
	else {
		printf("Content-Type: video/mp4\n");
		printf("Content-Length: %lu\n", mp4->head().boxsize);
		printf("\n");

		mp4_save_visitor v(stdout);
		mp4->accept(&v);
	}
}
