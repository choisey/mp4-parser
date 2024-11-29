/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_dump.h"
#include <stdio.h>

// mp4_dump_visitor

mp4_dump::mp4_dump_visitor::mp4_dump_visitor()
{
}

mp4_dump::mp4_dump_visitor::~mp4_dump_visitor()
{
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, std::vector<std::shared_ptr<mp4_abstract_box>>& boxes)
{
	switch ( head.boxtype ) {
		case MP4FILE:
			_pathvector.push_back("");
			break;

		default:
			{
				char boxtype[10];
				sprintf(boxtype, "%c%c%c%c",
						(char) ( head.boxtype / 0x1000000 ),
						(char) ( ( head.boxtype / 0x10000 ) & 0xff ),
						(char) ( ( head.boxtype / 0x100 ) & 0xff ),
						(char) ( head.boxtype & 0xff ));

				std::string dir = ( !_pathvector.empty() )
					? _pathvector.back() + boxtype
					: boxtype;
				_pathvector.push_back(dir + "/");
				printf("%s:%lu/%lu\n", dir.c_str(), head.offset, head.boxsize);
				break;
			}
	}

	for ( auto child: boxes ) {
		child->accept(this);
	}

	_pathvector.pop_back();
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, FileTypeBox& ftyp)
{
	printf("%sftyp:%lu/%lu major_brand=%s, minor_version=%s, compatible_brands=",
			_pathvector.back().c_str(), head.offset, head.boxsize,
			ftyp.major_brand.c_str(), ftyp.minor_version.c_str());
	int count = 0;
	for ( auto brand: ftyp.compatible_brands ) {
		if ( count++ ) printf(", ");
		printf("%s", brand.c_str());
	}
	printf("\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, MovieHeaderBox& mvhd)
{
	printf("%smvhd:%lu/%lu creation_time=%lu, modification_time=%lu, timescale=%u, duration=%lu, rate=0x0%x, volume=0x0%x, matrix={ ",
			_pathvector.back().c_str(), head.offset, head.boxsize,
			mvhd.creation_time, mvhd.modification_time, mvhd.timescale, mvhd.duration, mvhd.rate, mvhd.volume);
	int count = 0;
        for ( auto entry: mvhd.matrix ) {
		if ( count++ ) printf(", ");
                printf("0x0%x", entry);
        }
	printf(" }, next_track_ID=%u\n", mvhd.next_track_ID);
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, MovieExtendsHeaderBox& mhed)
{
	printf("%smhev:%lu/%lu fragment_duration=%lu\n",
			_pathvector.back().c_str(),
			head.offset, head.boxsize,
			mhed.fragment_duration);
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, TrackHeaderBox& tkhd)
{
	printf("%stkhd:%lu/%lu creation_time=%lu, modification_time=%lu, track_ID=%u, duration=%lu, volume=0x0%x, matrix={ ",
			_pathvector.back().c_str(), head.offset, head.boxsize,
			tkhd.creation_time, tkhd.modification_time, tkhd.track_ID, tkhd.duration, tkhd.volume);
	int count = 0;
        for ( auto entry: tkhd.matrix ) {
		if ( count++ ) printf(", ");
                printf("0x0%x", entry);
        }
	printf(" }, width=%u, height=%u\n", tkhd.width, tkhd.height);
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, MediaHeaderBox& mdhd)
{
	printf("%smdhd:%lu/%lu creation_time=%lu, modification_time=%lu, timescale=%u, duration=%lu, language=%c%c%c\n",
			_pathvector.back().c_str(), head.offset, head.boxsize,
			mdhd.creation_time, mdhd.modification_time, mdhd.timescale, mdhd.duration,
			mdhd.language[0], mdhd.language[1], mdhd.language[2]);
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, HandlerBox& hdlr)
{
	printf("%shdlr:%lu/%lu handler_type=%c%c%c%c, name=%s\n",
			_pathvector.back().c_str(), head.offset, head.boxsize,
			//hdlr.handler_type[0], hdlr.handler_type[1], hdlr.handler_type[2], hdlr.handler_type[3],
			(char) ( hdlr.handler_type / 0x1000000 ),
			(char) ( ( hdlr.handler_type / 0x10000 ) & 0xff ),
			(char) ( ( hdlr.handler_type / 0x100 ) & 0xff ),
			(char) ( hdlr.handler_type & 0xff ),
			hdlr.name.c_str());
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, VideoMediaHeaderBox& vmhd)
{
	printf("%svmhd:%lu/%lu graphicsmode=%u, opcolor[]={ %u, %u, %u }\n",
			_pathvector.back().c_str(), head.offset, head.boxsize,
			vmhd.graphicsmode, vmhd.opcolor[0], vmhd.opcolor[1], vmhd.opcolor[2]);
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, SoundMediaHeaderBox& smhd)
{
	printf("%ssmhd:%lu/%lu balance=%u\n",
			_pathvector.back().c_str(), head.offset, head.boxsize,
			smhd.balance);
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, HintMediaHeaderBox& hmhd)
{
	printf("%ssmhd:%lu/%lu maxPDUsize=%u, avgPDUsize=%u, maxbitrate=%u, avgbitrate=%u\n",
			_pathvector.back().c_str(), head.offset, head.boxsize,
			hmhd.maxPDUsize, hmhd.avgPDUsize, hmhd.maxbitrate, hmhd.avgbitrate);
}

//void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, SampleDescriptionBox::VisualSampleEntry& sd)
//{
//	std::string compressorname;
//	for ( size_t i = 0; i < sizeof(sd.compressorname); i++ ) {
//		compressorname += ( sd.compressorname[i] < ' ' ) ? '.' : sd.compressorname[i];
//	}
//
//	printf("%s%c%c%c%c:%lu/%lu ",
//			_pathvector.back().c_str(),
//			(char) ( head.boxtype / 0x1000000 ),
//			(char) ( ( head.boxtype / 0x10000 ) & 0xff ),
//			(char) ( ( head.boxtype / 0x100 ) & 0xff ),
//			(char) ( head.boxtype & 0xff ),
//			head.offset, head.boxsize);
//	printf("data_reference_index=%u, pre_defined=0x%02x, reserved=0x%02x, pre_defined={ 0x%04x, 0x%04x, 0x%04x }, ",
//			sd.data_reference_index,
//			sd.pre_defined1, sd.reserved1, sd.pre_defined2[0], sd.pre_defined2[1], sd.pre_defined2[2]);
//	printf("(width, height)=(%u, %u), (horizresolution, vertresolution)=(%u, %u), frame_count=%u, compressorname=%s, depth=%u, pre_defined=0x%04x\n",
//			sd.width, sd.height, sd.horizresolution, sd.vertresolution,
//			sd.frame_count, compressorname.c_str(), sd.depth, sd.pre_defined3);
//	if ( !sd.extensions.empty() ) {
//		for ( const auto& ext: sd.extensions ) {
//			printf("%s%c%c%c%c/%c%c%c%c:%lu/%lu {",
//					_pathvector.back().c_str(),
//					(char) ( head.boxtype / 0x1000000 ),
//					(char) ( ( head.boxtype / 0x10000 ) & 0xff ),
//					(char) ( ( head.boxtype / 0x100 ) & 0xff ),
//					(char) ( head.boxtype & 0xff ),
//					(char) ( ext->first.boxtype / 0x1000000 ),
//					(char) ( ( ext->first.boxtype / 0x10000 ) & 0xff ),
//					(char) ( ( ext->first.boxtype / 0x100 ) & 0xff ),
//					(char) ( ext->first.boxtype & 0xff ),
//					ext->first.offset, ext->first.boxsize);
//			int count = 0;
//			for ( auto byte: ext->second ) {
//				if ( count++ ) printf(",");
//				printf(" 0x%02x", byte);
//			}
//			printf(" }\n");
//		}
//	}
//}
//
//void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, SampleDescriptionBox::AudioSampleEntry& sd)
//{
//	printf("%s%c%c%c%c:%lu/%lu ",
//			_pathvector.back().c_str(),
//			(char) ( head.boxtype / 0x1000000 ),
//			(char) ( ( head.boxtype / 0x10000 ) & 0xff ),
//			(char) ( ( head.boxtype / 0x100 ) & 0xff ),
//			(char) ( head.boxtype & 0xff ),
//			head.offset, head.boxsize);
//	printf("data_reference_index=%u, reserved={ 0x%04x, 0x%04x }, ",
//			sd.data_reference_index, sd.reserved1[0], sd.reserved1[1]);
//	printf("channelcount=%u, samplesize=%u, pre_defined=0x%02x, reserved=0x%02x, samplerate=%u\n",
//			sd.channelcount, sd.samplesize, sd.pre_defined1, sd.reserved2, sd.samplerate);
//	if ( !sd.extensions.empty() ) {
//		for ( const auto& ext: sd.extensions ) {
//			printf("%s%c%c%c%c/%c%c%c%c:%lu/%lu {",
//					_pathvector.back().c_str(),
//					(char) ( head.boxtype / 0x1000000 ),
//					(char) ( ( head.boxtype / 0x10000 ) & 0xff ),
//					(char) ( ( head.boxtype / 0x100 ) & 0xff ),
//					(char) ( head.boxtype & 0xff ),
//					(char) ( ext->first.boxtype / 0x1000000 ),
//					(char) ( ( ext->first.boxtype / 0x10000 ) & 0xff ),
//					(char) ( ( ext->first.boxtype / 0x100 ) & 0xff ),
//					(char) ( ext->first.boxtype & 0xff ),
//					ext->first.offset, ext->first.boxsize);
//			int count = 0;
//			for ( auto byte: ext->second ) {
//				if ( count++ ) printf(",");
//				printf(" 0x%02x", byte);
//			}
//			printf(" }\n");
//		}
//	}
//}
//
//void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, SampleDescriptionBox::HintSampleEntry& sd)
//{
//	printf("%s%c%c%c%c:%lu/%lu data_reference_index=%u, data={",
//			_pathvector.back().c_str(),
//			(char) ( head.boxtype / 0x1000000 ),
//			(char) ( ( head.boxtype / 0x10000 ) & 0xff ),
//			(char) ( ( head.boxtype / 0x100 ) & 0xff ),
//			(char) ( head.boxtype & 0xff ),
//			head.offset, head.boxsize,
//			sd.data_reference_index);
//	int count = 0;
//	for ( auto byte: sd.data ) {
//		if ( count++ ) printf(",");
//		printf(" 0x%02x", byte);
//	}
//	printf(" }\n");
//}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, TimeToSampleBox& stts)
{
	printf("%sstts:%lu/%lu entry_count=%lu, (sample_count, sample_delta)={ ", _pathvector.back().c_str(), head.offset, head.boxsize, stts.entries.size());
	int count = 0;
	for ( auto e: stts.entries ) {
		if ( count++ ) printf(", ");
		printf("(%u, %u)", e.sample_count, e.sample_delta);
	}
	printf(" }\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, CompositionOffsetBox& ctts)
{
	printf("%sctts:%lu/%lu entry_count=%lu, (sample_count, sample_offset)={ ", _pathvector.back().c_str(), head.offset, head.boxsize, ctts.entries.size());
	int count = 0;
	for ( auto e: ctts.entries ) {
		if ( count++ ) printf(", ");
		printf("(%u, %u)", e.sample_count, e.sample_offset);
	}
	printf(" }\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, SampleToChunkBox& stsc)
{
	printf("%sstsc:%lu/%lu entry_count=%lu, (first_chunk, samples_per_chunk, sample_description_index)={ ",
			_pathvector.back().c_str(), head.offset, head.boxsize, stsc.entries.size());
	int count = 0;
	for ( auto e: stsc.entries ) {
		if ( count++ ) printf(", ");
		printf("(%u, %u, %u)", e.first_chunk, e.samples_per_chunk, e.sample_description_index);
	}
	printf(" }\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, SampleSizeBox& stsz)
{
	printf("%sstsz:%lu/%lu sample_size=%u, sample_count=%lu, entry_sizes={ ", _pathvector.back().c_str(), head.offset, head.boxsize, stsz.sample_size, stsz.entry_sizes.size());
	int count = 0;
	for ( auto size: stsz.entry_sizes ) {
		if ( count++ ) printf(", ");
		printf("%u", size);
	}
	printf(" }\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, ChunkOffsetBox& stco)
{
	printf("%sstco:%lu/%lu entry_count=%lu, chunk_offsets={ ", _pathvector.back().c_str(), head.offset, head.boxsize, stco.chunk_offsets.size());
	int count = 0;
	for ( auto co: stco.chunk_offsets ) {
		if ( count++ ) printf(", ");
		printf("%u", co);
	}
	printf(" }\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, ChunkLargeOffsetBox& co64)
{
	printf("%sco64:%lu/%lu entry_count=%lu, chunk_offsets={ ", _pathvector.back().c_str(), head.offset, head.boxsize, co64.chunk_offsets.size());
	int count = 0;
	for ( auto co: co64.chunk_offsets ) {
		if ( count++ ) printf(", ");
		printf("%lu", co);
	}
	printf(" }\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, SyncSampleBox& stss)
{
	printf("%sstss:%lu/%lu entry_count=%lu, sample_numbers={ ", _pathvector.back().c_str(), head.offset, head.boxsize, stss.sample_numbers.size());
	int count = 0;
	for ( auto s: stss.sample_numbers ) {
		if ( count++ ) printf(", ");
		printf("%u", s);
	}
	printf(" }\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, SampleDependencyTypeBox& sdtp)
{
	printf("%ssdtp:%lu/%lu entry_count=%lu, sample_dependencies={ ", _pathvector.back().c_str(), head.offset, head.boxsize, sdtp.sample_dependencies.size());
	int count = 0;
	for ( auto sd: sdtp.sample_dependencies ) {
		if ( count++ ) printf(", ");
		printf("(%u,%u,%u)", ((sd / 0x10) & 0x02), ((sd / 0x04) & 0x02), (sd & 0x02));
	}
	printf(" }\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, EditListBox& elst)
{
	printf("%selst:%lu/%lu entry_count=%lu, (segment_duration, media_time, media_rate_integer, media_rate_fraction)={ ", _pathvector.back().c_str(), head.offset, head.boxsize, elst.entries.size());
	int count = 0;
	for ( auto e: elst.entries ) {
		if ( count++ ) printf(", ");
		printf("(%lu, %ld, %d, %d)", e.segment_duration, e.media_time, e.media_rate_integer, e.media_rate_fraction);
	}
	printf(" }\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, TrackFragmentHeaderBox& tfhd)
{
	printf("%stfhd:%lu/%lu flag=0x%x, track_id=%u", _pathvector.back().c_str(),
			head.offset, head.boxsize,
			head.flag, tfhd.track_ID);

        if ( 0x000001 & head.flag ) {
                printf(", base_data_offset=%lu", tfhd.base_data_offset);
        }

        if ( 0x000002 & head.flag ) {
                printf(", sample_description_index=%u", tfhd.sample_description_index);
        }

        if ( 0x000008 & head.flag ) {
                printf(", default_sample_duration=%u", tfhd.default_sample_duration);
        }

        if ( 0x000010 & head.flag ) {
                printf(", default_sample_size=%u", tfhd.default_sample_size);
        }

        if ( 0x000020 & head.flag ) {
                printf(", default_sample_flags=%u", tfhd.default_sample_flags);
        }

	printf("\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, TrackFragmentDecodeTimeBox& tfdt)
{
	printf("%stfdt:%lu/%lu decode_time=%lu\n", _pathvector.back().c_str(),
			head.offset, head.boxsize,
			tfdt.decode_time);
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, TrackRunBox& trun)
{
	printf("%strun:%lu/%lu flag=0x%x, ", _pathvector.back().c_str(), head.offset, head.boxsize, head.flag);

	if ( 0x000001 & head.flag ) {
		printf("data_offset=%u, ", trun.data_offset);
	}

	if ( 0x000004 & head.flag ) {
		printf("first_sample_flags=0x%x, ", trun.first_sample_flags);
	}

	bool sample_duration_present = false;
	bool sample_size_present = false;
	bool sample_flags_present = false;
	bool sample_composition_time_offset_present = false;
	int sample_entyr_size = 0;
	int count = 0;

	printf("sample_count=%lu, ( ", trun.samples.size());

	if ( 0x000100 & head.flag ) {
		sample_duration_present = true;
		sample_entyr_size++;
		if ( count++ ) printf(", ");
		printf("sample_duration");
	}

	if ( 0x000200 & head.flag ) {
		sample_size_present = true;
		sample_entyr_size++;
		if ( count++ ) printf(", ");
		printf("sample_size");
	}

	if ( 0x000400 & head.flag ) {
		sample_flags_present = true;
		sample_entyr_size++;
		if ( count++ ) printf(", ");
		printf("sample_flags");
	}

	if ( 0x000800 & head.flag ) {
		sample_composition_time_offset_present = true;
		sample_entyr_size++;
		if ( count++ ) printf(", ");
		printf("sample_composition_time_offset");
	}

	printf(" )={ ");
	count = 0;

	for ( auto e: trun.samples ) {
		if ( count++ ) printf(", ");
		printf("{ ");
		int cnt_2 = 0;

		if ( sample_duration_present ) {
			if ( cnt_2++ ) printf(", ");
			printf("%u", e.sample_duration);
		}

		if ( sample_size_present ) {
			if ( cnt_2++ ) printf(", ");
			printf("%u", e.sample_size);
		}

		if ( sample_flags_present ) {
			if ( cnt_2++ ) printf(", ");
			printf("0x%x", e.sample_flags);
		}

		if ( sample_composition_time_offset_present ) {
			if ( cnt_2++ ) printf(", ");
			printf("%u", e.sample_composition_time_offset);
		}

		printf(" }");
	}

	printf(" }\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, SegmentIndexBox& sidx)
{
	printf("%ssidx:%lu/%lu reference_id=%u, timescale=%u, earliest_presentation_time=%lu, first_offset=%lu, entry_count=%lu, (reference_type, reference_size, subsequent_duration, contains_SAP, SAP_delta_time)={ ",
			_pathvector.back().c_str(),
			head.offset, head.boxsize,
			sidx.reference_id, sidx.timescale, sidx.earliest_presentation_time, sidx.first_offset, sidx.references.size());
	int count = 0;
	for ( auto e: sidx.references ) {
		if ( count++ ) printf(", ");
		printf("(%d, %u, %u, %d, %u)", e.reference_type, e.reference_size, e.subsegment_duration, e.contains_SAP, e.SAP_delta_time);
	}
	printf(" }\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, MovieFragmentHeaderBox& mfhd)
{
	printf("%smfhd:%lu/%lu sequence_number=%u\n",
			_pathvector.back().c_str(),
			head.offset, head.boxsize,
			mfhd.sequence_number);
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, TrackFragmentRandomAccessBox& tfra)
{
	printf("%stfra:%lu/%lu track_ID=%u, (time, moof_offset, #traf, #trun, #sample)={ ",
			_pathvector.back().c_str(),
			head.offset, head.boxsize,
			tfra.track_ID);
	int count = 0;
	for ( auto& e: tfra.entries ) {
		if ( count++ ) printf(", ");
		printf("(%lu, %lu, %u, %u, %u)", e.first, e.second.moof_offset, e.second.traf_number, e.second.trun_number, e.second.sample_number);
	}
	printf(" }\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, MovieFragmentRandomAccessOffsetBox& mfro)
{
	printf("%smfro:%lu/%lu size=%u\n",
			_pathvector.back().c_str(),
			head.offset, head.boxsize,
			mfro.size);
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, TrackExtendsBox& trex)
{
	printf("%strex:%lu/%lu track_ID=%u, default_sample_description_index=%u, default_sample_duration=%u, default_sample_size=%u, default_sample_flags=%u\n",
			_pathvector.back().c_str(),
			head.offset, head.boxsize,
			trex.track_ID,
			trex.default_sample_description_index,
			trex.default_sample_duration,
			trex.default_sample_size,
			trex.default_sample_flags);
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, MediaDataBox& mdat)
{
	printf("%smdat:%lu/%lu uri=%s, size=%lu, byte_ranges={ ", _pathvector.back().c_str(), head.offset, head.boxsize, mdat.uri.c_str(), mdat.size());

	int count = 0;
	for ( auto b: mdat.byte_ranges ) {
		if ( count++ ) printf(", ");
		printf("%lu-%lu/%u", b.first, b.first + b.second, b.second);
	}
	printf(" }, chunks={ ");

	count = 0;
	for ( auto c: mdat.chunks ) {
		if ( count++ ) printf(", ");
		printf("%lu-%lu/%u", c.first, c.first + c.second, c.second);
	}
	printf(" }\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, DataBox<std::vector<uint8_t>>& box)
{
	printf("%s%c%c%c%c:%lu/%lu { ",
			_pathvector.back().c_str(),
			(char) ( head.boxtype / 0x1000000 ),
			(char) ( ( head.boxtype / 0x10000 ) & 0xff ),
			(char) ( ( head.boxtype / 0x100 ) & 0xff ),
			(char) ( head.boxtype & 0xff ),
			head.offset, head.boxsize);
	int count = 0;
	for ( auto byte: box.boxdata ) {
		if ( count++ ) printf(", ");
		printf("0x%02x", byte);
	}
	printf(" }\n");
}

void mp4_dump::mp4_dump_visitor::visit(BoxHead& head, EmptyBox&)
{
	printf("%s%c%c%c%c:%lu/%lu\n",
			_pathvector.back().c_str(),
			(char) ( head.boxtype / 0x1000000 ),
			(char) ( ( head.boxtype / 0x10000 ) & 0xff ),
			(char) ( ( head.boxtype / 0x100 ) & 0xff ),
			(char) ( head.boxtype & 0xff ),
			head.offset, head.boxsize);
}

// mp4_dump

mp4_dump::mp4_dump()
{
}

mp4_dump::~mp4_dump()
{
}

void mp4_dump::execute(std::shared_ptr<mp4_abstract_box> box)
{
	box->accept(&_v);
}
