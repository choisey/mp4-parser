/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_relocate.h"
#include "mp4.h"
#include <assert.h>

mp4_relocate::mp4_relocate_visitor::mp4_relocate_visitor(size_t offset)
	: _offset(offset)
{
}

mp4_relocate::mp4_relocate_visitor::~mp4_relocate_visitor()
{
}

void mp4_relocate::mp4_relocate_visitor::relocate_head(BoxHead& head)
{
#ifdef _DEBUG
	fprintf(stderr, "** %s:%s(%d) head.boxtype=%c%c%c%c, head.boxheadsize=%lu, head.offset=%lu=>%lu\n", __func__, __FILE__, __LINE__,
			( head.boxtype & 0xff000000 ) / 0x1000000,
			( head.boxtype & 0xff0000 ) / 0x10000,
			( head.boxtype & 0xff00 ) / 0x100,
			head.boxtype & 0xff,
			head.boxheadsize,
			head.offset,
			_offset );
#endif

	head.offset = _offset;
	head.boxheadsize = head.size();
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, std::vector<std::shared_ptr<mp4_abstract_box>>& boxes)
{
	assert( _XX_ != head.boxtype );

	if ( MP4FILE != head.boxtype ) {
		relocate_head(head);
		_offset += head.boxheadsize;
	}

	size_t boxsize = 0;

	switch ( head.boxtype ) {
		case DREF:
		case STSD:
			// dref and stsd has a 32-bit entry_count after head
			boxsize += sizeof(uint32_t);
			_offset += sizeof(uint32_t);
			break;
	}

	for ( auto child: boxes ) {
		if ( _XX_ != child->head().boxtype ) {
			child->accept(this);
			boxsize += child->head().boxsize;
		}
	}

	head.boxsize = head.boxheadsize + boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, FileTypeBox& ftyp)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + ftyp.size();
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, MovieHeaderBox& mvhd)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + mvhd.size( head.version );
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, MovieExtendsHeaderBox& mehd)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + mehd.size ( head.version );
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, TrackHeaderBox& tkhd)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + tkhd.size( head.version );
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, MediaHeaderBox& mdhd)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + mdhd.size( head.version );
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, HandlerBox& hdlr)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + hdlr.size();
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, VideoMediaHeaderBox& vmhd)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + vmhd.size();
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, SoundMediaHeaderBox& smhd)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + smhd.size();
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, HintMediaHeaderBox& hmhd)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + hmhd.size();
	_offset += head.boxsize;
}

//void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, SampleDescriptionBox::VisualSampleEntry& sd)
//{
//	relocate_head(head);
//	head.boxsize = head.boxheadsize
//		+ sizeof(uint8_t) * 6				// reserved = 0
//		+ sizeof(uint16_t)				// data_reference_index
//		+ sizeof(uint16_t) * 2 + sizeof(uint32_t) * 3	// pre_defined, reserved
//		+ sizeof(uint16_t) * 2				// width, height
//		+ sizeof(uint32_t) * 2				// horizresolution, vertresolution
//		+ sizeof(uint32_t)
//		+ sizeof(uint16_t)				// frame_count
//		+ sizeof(uint8_t) * 32				// compressorname
//		+ sizeof(uint16_t) * 2;
//	assert( head.boxheadsize + 78 == head.boxsize );
//
//	for ( const auto& ext: sd.extensions ) {
//		head.boxsize += sizeof(uint32_t) * 2 + ext->second.size();
//	}
//
//	_offset += head.boxsize;
//}
//
//void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, SampleDescriptionBox::AudioSampleEntry& sd)
//{
//	relocate_head(head);
//	head.boxsize = head.boxheadsize
//		+ sizeof(uint8_t) * 6				// reserved = 0
//		+ sizeof(uint16_t)				// data_reference_index
//		+ sizeof(uint32_t) * 2				// reserved
//		+ sizeof(uint16_t)				// channelcount
//		+ sizeof(uint16_t)				// samplesize
//		+ sizeof(uint16_t) * 2				// pre_defined, reserved
//		+ sizeof(uint32_t);				// samplerate
//	assert( head.boxheadsize + 28 == head.boxsize );
//
//	for ( const auto& ext: sd.extensions ) {
//		head.boxsize += sizeof(uint32_t) * 2 + ext->second.size();
//	}
//
//	_offset += head.boxsize;
//}
//
//void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, SampleDescriptionBox::HintSampleEntry& sd)
//{
//	relocate_head(head);
//	head.boxsize = head.boxheadsize
//		+ sizeof(uint8_t) * 6				// reserved = 0
//		+ sizeof(uint16_t)				// data_reference_index
//		+ sd.data.size();
//	assert( head.boxheadsize + 8 + sd.data.size() == head.boxsize );
//	_offset += head.boxsize;
//}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, TimeToSampleBox& stts)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + stts.size();
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, CompositionOffsetBox& ctts)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + ctts.size();
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, SampleToChunkBox& stsc)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + stsc.size();
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, SampleSizeBox& stsz)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + stsz.size();
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, ChunkOffsetBox& stco)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + stco.size();
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, ChunkLargeOffsetBox& co64)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + co64.size();
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, SyncSampleBox& stss)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + stss.size();
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, SampleDependencyTypeBox& sdtp)
{
	head.boxheadsize = 0;
	head.boxsize = 0;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, EditListBox& elst)
{
	relocate_head(head);
	head.version = 1;
	head.boxsize = head.boxheadsize + elst.size( head.version );
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, TrackFragmentHeaderBox& tfhd)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + tfhd.size( head.flag );
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, TrackFragmentDecodeTimeBox& tfdt)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + tfdt.size();
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, TrackRunBox& trun)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + trun.size( head.flag );
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, SegmentIndexBox& sidx)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + sidx.size( head.version );
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, MovieFragmentHeaderBox& mfhd)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + mfhd.size();
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, TrackExtendsBox& trex)
{
	relocate_head(head);
	head.boxsize = head.boxheadsize + trex.size();
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, MediaDataBox& mdat)
{
	if ( !mdat.byte_ranges.empty() ) {
		relocate_head(head);
		head.boxsize = head.boxheadsize;

		for ( auto b: mdat.byte_ranges ) {
			head.boxsize += b.second;
		}

		_offset += head.boxsize;
	}
	else {
		head.boxheadsize = 0;
		head.boxsize = 0;
	}
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, DataBox<std::vector<uint8_t>>& box)
{
	relocate_head(head);
	_offset += head.boxsize;
}

void mp4_relocate::mp4_relocate_visitor::visit(BoxHead& head, EmptyBox& e)
{
	head.boxheadsize = 0;
	head.boxsize = 0;
}

// mp4_relocate

mp4_relocate::mp4_relocate(size_t offset)
	: _offset(offset)
{
}

mp4_relocate::~mp4_relocate()
{
}

void mp4_relocate::execute(std::shared_ptr<mp4_abstract_box> box)
{
	mp4_relocate_visitor relocator(_offset);
	box->accept(&relocator);
}

void Relocate(std::shared_ptr<mp4_abstract_box> box, size_t offset)
{
	mp4_relocate relocator(offset);
	relocator.execute(box);
}
