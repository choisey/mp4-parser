/*
 * Copyright (c) Seungyeob Choi
 */

// MP4 parser
// based on ISO/IEC 14496-12:2005(E)

#include "mp4_load.h"
#include "mp4.h"
#include "mp4_concrete_box.h"
#include "mp4_container_box.h"
#include "mp4_file.h"
#include "io.h"
#include <assert.h>
#include <map>

// mp4_load_visitor

mp4_load::mp4_load_visitor::mp4_load_visitor(std::shared_ptr<io_file> f)
	: _f(f)
{
	assert( _f->is_open() );
}

mp4_load::mp4_load_visitor::~mp4_load_visitor()
{
	assert( !_f->is_open() );
}

bool mp4_load::mp4_load_visitor::read_i32(int& i32)
{
	uint8_t buf[4];
	if ( _f->read(buf, sizeof(buf)) == sizeof(buf) ) {
		i32 = I32(buf[0], buf[1], buf[2], buf[3]);
		return true;
	}

	return false;
}

bool mp4_load::mp4_load_visitor::read_u8(uint8_t& u8)
{
	uint8_t buf[1];
	if ( _f->read(buf, sizeof(buf)) == sizeof(buf) ) {
		u8 = buf[0];
		return true;
	}

	return false;
}

bool mp4_load::mp4_load_visitor::read_u32(uint32_t& u32)
{
	uint8_t buf[4];
	if ( _f->read(buf, sizeof(buf)) == sizeof(buf) ) {
		u32 = U32(buf[0], buf[1], buf[2], buf[3]);
		return true;
	}

	return false;
}

bool mp4_load::mp4_load_visitor::read_u64(uint64_t& u64)
{
	uint8_t buf[8];
	if ( _f->read(buf, sizeof(buf)) == sizeof(buf) ) {
		u64 = U64(buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
		return true;
	}

	return false;
}

bool mp4_load::mp4_load_visitor::read_box_head(BoxHead& boxhead)
{
	assert( _f->is_open() );

	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	// |    size(32)   |    type(32)   |
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

	uint8_t buf[8];
	boxhead.boxheadsize = sizeof(buf);
	if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
		return false;
	}

	boxhead.boxsize = U32(buf[0], buf[1], buf[2], buf[3]);
	boxhead.boxtype = U32(buf[4], buf[5], buf[6], buf[7]);

//	uint32_t boxsize;
//	uint32_t boxtype;
//
//	if ( !read_u32(_fp, boxsize) ) {
//		boxhead.boxheadsize = 0;
//		boxhead.boxsize = 0;
//		return false;
//	}
//
//	// occasionally we get 0x0000
//	if ( boxsize <= sizeof(boxsize) || !read_u32(_fp, boxtype) ) {
//		boxhead.boxheadsize = sizeof(boxsize);
//		boxhead.boxsize = sizeof(boxsize);
//		return false;
//	}
//
//	boxhead.boxheadsize = sizeof(boxsize) + sizeof(boxtype);
//	boxhead.boxsize = boxsize;
//	boxhead.boxtype = boxtype;

	// other boxes
	// if ( size==1 )
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	// |         largesize(64)         |
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

	if ( 1 == boxhead.boxsize ) {
		boxhead.boxheadsize += 8;
		if ( !read_u64(boxhead.boxsize) ) {
			return false;
		}
	}
	else if ( 0 == boxhead.boxsize ) {
		// box extends to the end of file...
		boxhead.boxsize = _f->size() - _f->position();
	}

	// if ( type=="uuid" )
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	// |     usertype(8)[16]...        |
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

	if ( UUID == boxhead.boxtype ) {
		char extended_type[16];

		if ( _f->read(extended_type, sizeof(extended_type)) != sizeof(extended_type) ) {
			return false;
		}

		boxhead.boxheadsize += sizeof(extended_type);
		std::string usertype = std::string((const char*) extended_type, 16);
	}

	// FullBox
	// +-+-+-+-+-+-+-+
	// |v| flags(24) |
	// +-+-+-+-+-+-+-+

	if ( boxhead.is_fullbox() ) {
		uint8_t fbbuf[4];
		if ( _f->read(fbbuf, sizeof(fbbuf)) != sizeof(fbbuf) ) {
			return false;
		}
		boxhead.version = fbbuf[0];
		boxhead.flag = fbbuf[1] * 0x10000
			+ fbbuf[2] * 0x100
			+ fbbuf[3];
		boxhead.boxheadsize += sizeof(fbbuf);
	}

	return true;
}

size_t mp4_load::mp4_load_visitor::read_box(size_t offset, uint32_t ctype, std::vector<std::shared_ptr<mp4_abstract_box>>& boxes)
{
	assert( _f->is_open() );

	BoxHead head;
	head.offset = offset;

	if ( !read_box_head(head) ) {
		return 0;
	}

	// free, skip

	switch ( head.boxtype ) {
		case FREE:
		case SKIP:
			{
				auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
				b->accept(this);
				boxes.push_back(b);
				return head.boxsize;
			}
	}

	// meta

	switch ( ctype ) {
		case MP4FILE:
		case MOOV:
		case TRAK:
			switch ( head.boxtype ) {
				case META:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						return head.boxsize;
					}
			}
			break;

		case UDTA:
			switch ( head.boxtype ) {
				case META:
					{
						auto b = new_box<mp4_container_box>(head);
						b->accept(this);
						boxes.push_back(b);
						return head.boxsize;
					}
			}
			break;
	}

	// other boxes

	switch ( ctype )
	{
		case MP4FILE:
			switch ( head.boxtype )
			{
				case FTYP:
					{
						auto b = new_box<mp4_concrete_box<FileTypeBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case MFRA:
				case MOOF:
				case MOOV:
					{
						auto b = new_box<mp4_container_box>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case MDAT:
					{
						auto b = new_box<mp4_concrete_box<MediaDataBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case ABST:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case SIDX:
					{
						auto b = new_box<mp4_concrete_box<SegmentIndexBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				default:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}
			}
			break;

		case MOOV:
			switch ( head.boxtype ) {
				case MVHD:
					{
						auto b = new_box<mp4_concrete_box<MovieHeaderBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case MVEX:
				case TRAK:
				case UDTA:
					{
						auto b = new_box<mp4_container_box>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				default:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}
			}
			break;

		case TRAK:
			switch ( head.boxtype ) {
				case TKHD:
					{
						auto b = new_box<mp4_concrete_box<TrackHeaderBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case EDTS:
				case MDIA:
				case UDTA:
					{
						auto b = new_box<mp4_container_box>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case TREF:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				default:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}
			}
			break;

		case MDIA:
			switch ( head.boxtype ) {
				case MDHD:
					{
						auto b = new_box<mp4_concrete_box<MediaHeaderBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case HDLR:
					{
						auto b = new_box<mp4_concrete_box<HandlerBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case MINF:
					{
						auto b = new_box<mp4_container_box>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				default:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}
			}
			break;

		case MINF:
			switch ( head.boxtype ) {
				case DINF:
				case STBL:
					{
						auto b = new_box<mp4_container_box>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case VMHD:
					{
						auto b = new_box<mp4_concrete_box<VideoMediaHeaderBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case SMHD:
					{
						auto b = new_box<mp4_concrete_box<SoundMediaHeaderBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case HMHD:
					{
						auto b = new_box<mp4_concrete_box<HintMediaHeaderBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case NMHD:
					{
						auto b = new_box<mp4_concrete_box<DataBox<std::vector<uint8_t>>>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				default:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}
			}
			break;

		case STBL:
			switch ( head.boxtype ) {
				case STSD:
					{
						auto b = new_box<mp4_container_box>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case STTS:
					{
						auto b = new_box<mp4_concrete_box<TimeToSampleBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case CTTS:
					{
						auto b = new_box<mp4_concrete_box<CompositionOffsetBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case STSC:
					{
						auto b = new_box<mp4_concrete_box<SampleToChunkBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case STSZ:
					{
						auto b = new_box<mp4_concrete_box<SampleSizeBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case STCO:
					{
						auto b = new_box<mp4_concrete_box<ChunkOffsetBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case CO64:
					{
						auto b = new_box<mp4_concrete_box<ChunkLargeOffsetBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case STSS:
					{
						auto b = new_box<mp4_concrete_box<SyncSampleBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case SDTP:
					{
						auto b = new_box<mp4_concrete_box<SampleDependencyTypeBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				default:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}
			}
			break;

		case STSD:
			{
				if ( !_handler_types.empty() ) {
					switch ( _handler_types.back() ) {
						case HandlerBox::VIDEO:
							{
								//auto b = new_box<mp4_concrete_box<SampleDescriptionBox::VisualSampleEntry>>(head);
								// To be safe
								auto b = new_box<mp4_concrete_box<DataBox<std::vector<uint8_t>>>>(head);
								b->accept(this);
								boxes.push_back(b);
								break;
							}
						case HandlerBox::AUDIO:
							{
								//auto b = new_box<mp4_concrete_box<SampleDescriptionBox::AudioSampleEntry>>(head);
								// To be safe
								auto b = new_box<mp4_concrete_box<DataBox<std::vector<uint8_t>>>>(head);
								b->accept(this);
								boxes.push_back(b);
								break;
							}
						default:
							{
								auto b = new_box<mp4_concrete_box<DataBox<std::vector<uint8_t>>>>(head);
								b->accept(this);
								boxes.push_back(b);
							}
					}
				}
				else {
					assert(false);
					auto b = new_box<mp4_concrete_box<DataBox<std::vector<uint8_t>>>>(head);
					b->accept(this);
					boxes.push_back(b);
				}
				break;
			}

		case MOOF:
			switch ( head.boxtype ) {
				case MFHD:
					{
						auto b = new_box<mp4_concrete_box<MovieFragmentHeaderBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case TRAF:
					{
						auto b = new_box<mp4_container_box>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				default:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}
			}
			break;

		case TRAF:
			switch ( head.boxtype ) {
				case TFHD:
					{
						auto b = new_box<mp4_concrete_box<TrackFragmentHeaderBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case TFDT:
					{
						auto b = new_box<mp4_concrete_box<TrackFragmentDecodeTimeBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case TRUN:
					{
						auto b = new_box<mp4_concrete_box<TrackRunBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				default:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}
			}
			break;

		case EDTS:
			switch ( head.boxtype ) {
				case ELST:
					{
						auto b = new_box<mp4_concrete_box<EditListBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				default:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}
			}
			break;

		case DINF:
			switch ( head.boxtype ) {
				case DREF:
					{
						auto b = new_box<mp4_container_box>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case URL:
					{
						auto b = new_box<mp4_concrete_box<DataBox<std::vector<uint8_t>>>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case URN:
					{
						auto b = new_box<mp4_concrete_box<DataBox<std::vector<uint8_t>>>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				default:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}
			}
			break;

		case DREF:
			switch ( head.boxtype ) {
				case URL:
					{
						auto b = new_box<mp4_concrete_box<DataBox<std::vector<uint8_t>>>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case URN:
					{
						auto b = new_box<mp4_concrete_box<DataBox<std::vector<uint8_t>>>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				default:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}
			}
			break;

		case MVEX:
			switch ( head.boxtype ) {
				case MEHD:
					{
						auto b = new_box<mp4_concrete_box<MovieExtendsHeaderBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case TREX:
					{
						auto b = new_box<mp4_concrete_box<TrackExtendsBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}
			}
			break;

		case MFRA:
			switch ( head.boxtype ) {
				case TFRA:
					{
						auto b = new_box<mp4_concrete_box<TrackFragmentRandomAccessBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				case MFRO:
					{
						auto b = new_box<mp4_concrete_box<MovieFragmentRandomAccessOffsetBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				default:
					{
						auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}
			}
			break;

		case UDTA:
			{
				auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
				b->accept(this);
				boxes.push_back(b);
				break;
			}

		case META:
			switch ( head.boxtype ) {
				case HDLR:
					{
						auto b = new_box<mp4_concrete_box<HandlerBox>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}

				default:
					{
						auto b = new_box<mp4_concrete_box<DataBox<std::vector<uint8_t>>>>(head);
						b->accept(this);
						boxes.push_back(b);
						break;
					}
			}
			break;

		default:
			{
				assert(false);
				auto b = new_box<mp4_concrete_box<EmptyBox>>(head);
				b->accept(this);
				boxes.push_back(b);
				break;
			}
	}

	return head.boxsize;
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, std::vector<std::shared_ptr<mp4_abstract_box>>& boxes)
{
	assert( _f->is_open() );

	switch ( head.boxtype ) {
		case STSD:
			{
				uint8_t buf[4];
				if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
					return;
				}
				uint32_t entry_count = U32(buf[0], buf[1], buf[2], buf[3]);

				size_t offset = head.offset + head.boxheadsize + sizeof(uint32_t);
				size_t end_of_container = head.offset + head.boxsize;
				if ( end_of_container <= offset ) {
					return;
				}

				size_t nb;
				while ( ( nb = read_box(offset, head.boxtype, boxes) ) != 0 ) {
					offset += nb;
					if ( end_of_container <= offset ) {
						return;
					}
					_f->seek(offset, SEEK_SET);

					assert( 0 != entry_count-- );
				}
				break;
			}

		case DREF:
			{
				uint32_t entry_count;
				if ( !read_u32(entry_count) ) {
					return;
				}

				size_t offset = head.offset + head.boxheadsize + sizeof(uint32_t);
				size_t end_of_container = head.offset + head.boxsize;
				if ( end_of_container <= offset ) {
					return;
				}

				for ( uint32_t i = 0; i < entry_count; i++ ) {
					size_t nb = read_box(offset, head.boxtype, boxes);
					if ( 0 == nb ) {
						break;
					}

					offset += nb;
					if ( end_of_container <= offset ) {
						return;
					}

					_f->seek(offset, SEEK_SET);
				}
				assert( end_of_container == offset );
				break;
			}

		default:
			{
				size_t offset = head.offset + head.boxheadsize;
				size_t end_of_container = head.offset + head.boxsize;
				if ( end_of_container <= offset ) {
					return;
				}

				size_t nb;
				while ( ( nb = read_box(offset, head.boxtype, boxes) ) != 0 ) {
					offset += nb;
					if ( end_of_container <= offset ) {
						return;
					}
					_f->seek(offset, SEEK_SET);
				}
				break;
			}
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, FileTypeBox& ftyp)
{
	size_t size = head.boxsize - head.boxheadsize;
	uint8_t buf[size];
	if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
		return;
	}

	ftyp.major_brand = std::string((char*) buf, 4);
	ftyp.minor_version = std::string((char*) buf + 4, 4);

	int brands = size / 4 - 2;
	uint8_t* bp = buf + 8;
	for (int i = 0; i < brands; i++) {
		ftyp.compatible_brands.push_back(std::string((char*) bp, 4));
		bp += 4;
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, MovieHeaderBox& mvhd)
{
	if ( 1 == head.version ) {
		uint8_t buf[ mvhd.size(head.version) ];
		assert( 108 == sizeof(buf) );
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}

		mvhd.creation_time = U64(buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
		mvhd.modification_time = U64(buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
		mvhd.timescale = U32(buf[16], buf[17], buf[18], buf[19]);
		mvhd.duration = U64(buf[20], buf[21], buf[22], buf[23], buf[24], buf[25], buf[26], buf[27]);

		uint8_t* bp = buf + 28;
		mvhd.rate = U32(*bp, *(bp+1), *(bp+2), *(bp+3));
		mvhd.volume = U32(0, 0, *(bp+4), *(bp+5));
		bp += 16;
		for (int i = 0; i < 9; i++) {
			mvhd.matrix.push_back(U32(*bp, *(bp+1), *(bp+2), *(bp+3)));
			bp += 4;
		}
		bp += 24;
		mvhd.next_track_ID = U32(*bp, *(bp+1), *(bp+2), *(bp+3));
	}
	else { // version==0
		uint8_t buf[ mvhd.size(head.version) ];
		assert( 96 == sizeof(buf) );
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}

		mvhd.creation_time = U32(buf[0], buf[1], buf[2], buf[3]);
		mvhd.modification_time = U32(buf[4], buf[5], buf[6], buf[7]);
		mvhd.timescale = U32(buf[8], buf[9], buf[10], buf[11]);
		mvhd.duration = U32(buf[12], buf[13], buf[14], buf[15]);

		uint8_t* bp = buf + 16;
		mvhd.rate = U32(*bp, *(bp+1), *(bp+2), *(bp+3));
		mvhd.volume = U32(0, 0, *(bp+4), *(bp+5));
		bp += 16;
		for (int i = 0; i < 9; i++) {
			mvhd.matrix.push_back(U32(*bp, *(bp+1), *(bp+2), *(bp+3)));
			bp += 4;
		}
		bp += 24;
		mvhd.next_track_ID = U32(*bp, *(bp+1), *(bp+2), *(bp+3));
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, MovieExtendsHeaderBox& mehd)
{
	if ( 1 == head.version ) {
		uint8_t buf[8];
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}
		mehd.fragment_duration = U64(
				buf[0], buf[1], buf[2], buf[3],
				buf[4], buf[5], buf[6], buf[7]);
	}
	else {
		uint8_t buf[4];
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}
		mehd.fragment_duration = U32(buf[0], buf[1], buf[2], buf[3]);
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, TrackHeaderBox& tkhd)
{
	if ( 1 == head.version ) {
		uint8_t buf[ tkhd.size(head.version) ];
		assert( 92 == sizeof(buf) );
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}

		tkhd.creation_time = U64(buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
		tkhd.modification_time = U64(buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
		tkhd.track_ID = U32(buf[16], buf[17], buf[18], buf[19]);
		tkhd.duration = U64(buf[24], buf[25], buf[26], buf[27], buf[28], buf[29], buf[30], buf[31]);

		uint8_t* bp = buf + 44;
		tkhd.volume = U32(0, 0, *bp, *(bp+1));
		bp += 4;
		for (int i = 0; i < 9; i++) {
			tkhd.matrix.push_back(U32(*bp, *(bp+1), *(bp+2), *(bp+3)));
			bp += 4;
		}
		tkhd.width = U32(*bp, *(bp+1), *(bp+2), *(bp+3));
		tkhd.height = U32(*(bp+4), *(bp+5), *(bp+6), *(bp+7));
	}
	else { // version==0
		uint8_t buf[ tkhd.size(head.version) ];
		assert( 80 == sizeof(buf) );
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}

		tkhd.creation_time = U32(buf[0], buf[1], buf[2], buf[3]);
		tkhd.modification_time = U32(buf[4], buf[5], buf[6], buf[7]);
		tkhd.track_ID = U32(buf[8], buf[9], buf[10], buf[11]);
		tkhd.duration = U32(buf[16], buf[17], buf[18], buf[19]);

		uint8_t* bp = buf + 32;
		tkhd.volume = U32(0, 0, *bp, *(bp+1));
		bp += 4;
		for (int i = 0; i < 9; i++) {
			tkhd.matrix.push_back(U32(*bp, *(bp+1), *(bp+2), *(bp+3)));
			bp += 4;
		}
		tkhd.width = U32(*bp, *(bp+1), *(bp+2), *(bp+3));
		tkhd.height = U32(*(bp+4), *(bp+5), *(bp+6), *(bp+7));
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, MediaHeaderBox& mdhd)
{
	if ( 1 == head.version ) {
		uint8_t buf[ mdhd.size(head.version) ];
		assert( 32 == sizeof(buf) );
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}

		mdhd.creation_time = U64(buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
		mdhd.modification_time = U64(buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
		mdhd.timescale = U32(buf[16], buf[17], buf[18], buf[19]);
		mdhd.duration = U64(buf[20], buf[21], buf[22], buf[23], buf[24], buf[25], buf[26], buf[27]);
		mdhd.language[0] = 0x60 + ( ( buf[28] & 0x7C ) >> 2 );
		mdhd.language[1] = 0x60 + ( ( ( ( buf[28] * 0x100 ) | buf[29] ) & 0x3E0 ) >> 5 );
		mdhd.language[2] = 0x60 + ( buf[29] & 0x1F );
	}
	else { // version==0
		uint8_t buf[ mdhd.size(head.version) ];
		assert( 20 == sizeof(buf) );
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}

		mdhd.creation_time = U32(buf[0], buf[1], buf[2], buf[3]);
		mdhd.modification_time = U32(buf[4], buf[5], buf[6], buf[7]);
		mdhd.timescale = U32(buf[8], buf[9], buf[10], buf[11]);
		mdhd.duration = U32(buf[12], buf[13], buf[14], buf[15]);
		mdhd.language[0] = 0x60 + ( ( buf[16] & 0x7C ) >> 2 );
		mdhd.language[1] = 0x60 + ( ( ( ( buf[16] * 0x100 ) | buf[17] ) & 0x3E0 ) >> 5 );
		mdhd.language[2] = 0x60 + ( buf[17] & 0x1F );
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, HandlerBox& hdlr)
{
	uint8_t buf[ head.boxsize - head.boxheadsize ];
	if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
		return;
	}
	//uint32_t pre_defined = U32(buf[0], buf[1], buf[2], buf[3]);
	//uint32_t handler_type = U32(buf[4], buf[5], buf[6], buf[7]);
	//uint32_t reserved1 = U32(buf[8], buf[9], buf[10], buf[11]);
	//uint32_t reserved2 = U32(buf[12], buf[13], buf[14], buf[15]);
	//uint32_t reserved3 = U32(buf[16], buf[17], buf[18], buf[19]);
	hdlr.handler_type = U32(buf[4], buf[5], buf[6], buf[7]);
	hdlr.name = std::string((char*) buf + 20, sizeof(buf) - 20);

	_handler_types.push_back(hdlr.handler_type);
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, VideoMediaHeaderBox& vmhd)
{
	uint8_t buf[8];
	if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
		return;
	}
	vmhd.graphicsmode = U16(buf[0], buf[1]);
	vmhd.opcolor[0] = U16(buf[2], buf[3]);
	vmhd.opcolor[1] = U16(buf[4], buf[5]);
	vmhd.opcolor[2] = U16(buf[6], buf[7]);
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, SoundMediaHeaderBox& smhd)
{
	uint8_t buf[4];
	if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
		return;
	}
	smhd.balance = U16(buf[0], buf[1]);
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, HintMediaHeaderBox& hmhd)
{
	uint8_t buf[16];
	if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
		return;
	}
	hmhd.maxPDUsize = U16(buf[0], buf[1]);
	hmhd.avgPDUsize = U16(buf[2], buf[3]);
	hmhd.maxbitrate = U32(buf[4], buf[5], buf[6], buf[7]);
	hmhd.avgbitrate = U32(buf[8], buf[9], buf[10], buf[11]);
}

//void mp4_load::mp4_load_visitor::visit(BoxHead& head, SampleDescriptionBox::VisualSampleEntry& sd)
//{
//	uint8_t buf[78];
//	if ( 1 != fread(buf, sizeof(buf), 1, _fp) ) {
//		return;
//	}
//
//	sd.data_reference_index = U16(buf[6], buf[7]);
//
//	sd.pre_defined1 = U16(buf[8], buf[9]);
//	sd.reserved1 = U16(buf[10], buf[11]);
//	sd.pre_defined2[0] = U32(buf[12], buf[13], buf[14], buf[15]);
//	sd.pre_defined2[1] = U32(buf[16], buf[17], buf[18], buf[19]);
//	sd.pre_defined2[2] = U32(buf[20], buf[21], buf[22], buf[23]);
//
//	sd.width = U16(buf[24], buf[25]);
//	sd.height = U16(buf[26], buf[27]);
//	sd.horizresolution = U32(buf[28], buf[29], buf[30], buf[31]);
//	sd.vertresolution = U32(buf[32], buf[33], buf[34], buf[35]);
//
//	sd.reserved2 = U32(buf[36], buf[37], buf[38], buf[39]);
//
//	sd.frame_count = U16(buf[40], buf[41]);
//	assert( 1 == sd.frame_count );
//	memcpy(sd.compressorname, &buf[42], 32);
//	sd.depth = U16(buf[74], buf[75]);
//
//	sd.pre_defined3 = U16(buf[76], buf[77]);
//
//	assert( 0x0018 == sd.depth );
//	assert( 0xffff == sd.pre_defined3 );
//
//	size_t cur = head.boxheadsize + sizeof(buf);
//
//	while ( cur < head.boxsize ) {
//		BoxHead ext_head;
//
//		if ( !read_box_head(ext_head) ) {
//			cur += ext_head.boxsize;
//			break;
//		}
//
//		ext_head.offset = head.offset + cur;
//		cur += ext_head.boxheadsize;
//
//		auto ext = std::make_shared<SampleDescriptionBox::SampleDescriptionExtension>();
//		ext->first = ext_head;
//
//		uint8_t ext_buf[ ext_head.boxsize - ext_head.boxheadsize ];
//		if ( 1 == fread(ext_buf, sizeof(ext_buf), 1, _fp) ) {
//			ext->second.assign(ext_buf, ext_buf + sizeof(ext_buf));
//			cur += sizeof(ext_buf);
//		}
//
//		sd.extensions.push_back(ext);
//	}
//
//	assert( cur == head.boxsize );
//}
//
//void mp4_load::mp4_load_visitor::visit(BoxHead& head, SampleDescriptionBox::AudioSampleEntry& sd)
//{
//	uint8_t buf[28];
//	if ( 1 != fread(buf, sizeof(buf), 1, _fp) ) {
//		return;
//	}
//
//	sd.data_reference_index = U16(buf[6], buf[7]);
//
//	sd.reserved1[0] = U32(buf[8], buf[9], buf[10], buf[11]);
//	sd.reserved1[1] = U32(buf[12], buf[13], buf[14], buf[15]);
//
//	sd.channelcount = U16(buf[16], buf[17]);
//	sd.samplesize = U16(buf[18], buf[19]);
//
//	sd.pre_defined1 = U16(buf[20], buf[21]);
//	sd.reserved2 = U16(buf[22], buf[23]);
//
//	sd.samplerate = U32(buf[24], buf[25], buf[26], buf[27]);
//
//	size_t cur = head.boxheadsize + sizeof(buf);
//
//	while ( cur < head.boxsize ) {
//		BoxHead ext_head;
//
//		if ( !read_box_head(ext_head) ) {
//			cur += ext_head.boxsize;
//			break;
//		}
//
//		ext_head.offset = head.offset + cur;
//		cur += ext_head.boxheadsize;
//
//		auto ext = std::make_shared<SampleDescriptionBox::SampleDescriptionExtension>();
//		ext->first = ext_head;
//
//		uint8_t ext_buf[ ext_head.boxsize - ext_head.boxheadsize ];
//		if ( 1 == fread(ext_buf, sizeof(ext_buf), 1, _fp) ) {
//			ext->second.assign(ext_buf, ext_buf + sizeof(ext_buf));
//			cur += sizeof(ext_buf);
//		}
//
//		sd.extensions.push_back(ext);
//	}
//
//	assert( cur == head.boxsize );
//}
//
//void mp4_load::mp4_load_visitor::visit(BoxHead& head, SampleDescriptionBox::HintSampleEntry& sd)
//{
//	size_t size = head.boxsize - head.boxheadsize;
//	uint8_t buf[size];
//	if ( 1 != fread(buf, sizeof(buf), 1, _fp) ) {
//		return;
//	}
//	sd.data.assign(buf, buf + size);
//}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, TimeToSampleBox& stts)
{
	uint32_t entry_count;
	if ( !read_u32(entry_count) ) {
		return;
	}
	for ( uint32_t i = 0; i < entry_count; i++ ) {
		uint8_t buf[8];
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}
		stts.entries.push_back( {
				U32(buf[0], buf[1], buf[2], buf[3]),
				U32(buf[4], buf[5], buf[6], buf[7])
				} );
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, CompositionOffsetBox& ctts)
{
	uint32_t entry_count;
	if ( !read_u32(entry_count) ) {
		return;
	}
	for ( uint32_t i = 0; i < entry_count; i++ ) {
		uint8_t buf[8];
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}
		ctts.entries.push_back( {
				U32(buf[0], buf[1], buf[2], buf[3]),
				U32(buf[4], buf[5], buf[6], buf[7])
				} );
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, SampleToChunkBox& stsc)
{
	uint32_t entry_count;
	if ( !read_u32(entry_count) ) {
		return;
	}
	for ( uint32_t i = 0; i < entry_count; i++ ) {
		uint8_t buf[12];
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}
		stsc.entries.push_back( {
				U32(buf[0], buf[1], buf[2], buf[3]),
				U32(buf[4], buf[5], buf[6], buf[7]),
				U32(buf[8], buf[9], buf[10], buf[11])
				} );
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, SampleSizeBox& stsz)
{
	uint8_t buf[8];
	if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
		return;
	}
	stsz.sample_size = U32(buf[0], buf[1], buf[2], buf[3]);
	uint32_t sample_count = U32(buf[4], buf[5], buf[6], buf[7]);
	if ( 0 == stsz.sample_size ) {
		for ( uint32_t i = 0; i < sample_count; i++ ) {
			uint32_t entry_size;
			if ( !read_u32(entry_size) ) {
				return;
			}
			stsz.entry_sizes.push_back(entry_size);
		}
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, ChunkOffsetBox& stco)
{
	uint32_t entry_count;
	if ( !read_u32(entry_count) ) {
		return;
	}
	for ( uint32_t i = 0; i < entry_count; i++ ) {
		uint32_t chunk_offset;
		if ( !read_u32(chunk_offset) ) {
			return;
		}
		stco.chunk_offsets.push_back(chunk_offset);
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, ChunkLargeOffsetBox& co64)
{
	uint32_t entry_count;
	if ( !read_u32(entry_count) ) {
		return;
	}
	for ( uint32_t i = 0; i < entry_count; i++ ) {
		uint64_t chunk_offset;
		if ( !read_u64(chunk_offset) ) {
			return;
		}
		co64.chunk_offsets.push_back(chunk_offset);
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, SyncSampleBox& stss)
{
	uint32_t entry_count;
	if ( !read_u32(entry_count) ) {
		return;
	}
	for ( uint32_t i = 0; i < entry_count; i++ ) {
		uint32_t sample_number;
		if ( !read_u32(sample_number) ) {
			return;
		}
		stss.sample_numbers.push_back(sample_number);
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, SampleDependencyTypeBox& sdtp)
{
	uint32_t entry_count = ( head.boxsize - head.boxheadsize ) / sizeof(uint8_t);;

	for ( uint32_t i = 0; i < entry_count; i++ ) {
		uint8_t sample_dependency;
		if ( !read_u8(sample_dependency) ) {
			return;
		}
		sdtp.sample_dependencies.push_back(sample_dependency);
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, EditListBox& elst)
{
	uint32_t entry_count;
	if ( !read_u32(entry_count) ) {
		return;
	}
	if ( 1 == head.version ) {
		for ( uint32_t i = 0; i < entry_count; i++ ) {
			uint8_t buf[20];
			if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
				return;
			}
			elst.entries.push_back( {
					U64(buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]),
					I64(buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]),
					I16(buf[16], buf[17]),
					I16(buf[18], buf[19])
					} );
		}
	}
	else { // version==0
		for ( uint32_t i = 0; i < entry_count; i++ ) {
			uint8_t buf[12];
			if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
				return;
			}
			elst.entries.push_back( {
					U32(buf[0], buf[1], buf[2], buf[3]),
					I32(buf[4], buf[5], buf[6], buf[7]),
					I16(buf[8], buf[9]),
					I16(buf[10], buf[11])
					} );
		}
	}
}

/*
void mp4_load::mp4_load_visitor::visit(BoxHead& head, DataReferenceBox& dref)
{
	uint32_t entry_count;
	if ( !read_u32(entry_count) ) {
		return;
	}

	BoxHead dataentry_head;
	dataentry_head.offset = head.offset + head.boxheadsize + sizeof(uint32_t);

	for ( uint32_t i = 0; i < entry_count; i++ ) {
		if ( !read_box_head(dataentry_head) ) {
			return;
		}

		switch ( dataentry_head.boxtype )
		{
			case URL:
				{
					uint8_t buf[dataentry_head.boxsize - dataentry_head.boxheadsize];
					if ( 1 != fread(buf, sizeof(buf), 1, _fp) ) {
						return;
					}
					// TODO

				}
				break;

			case URN:
				{
					uint8_t buf[dataentry_head.boxsize - dataentry_head.boxheadsize];
					if ( 1 != fread(buf, sizeof(buf), 1, _fp) ) {
						return;
					}
					// TODO
				}
				break;

			case URL:
			case URN:
				{
					auto b = new_box<mp4_concrete_box<DataBox<std::vector<uint8_t>>>>(dataentry_head);
					b->accept(this);
					//boxes.push_back(b);
std::cerr << "DREF" << std::endl;
					break;
				}

			default:
				assert( false );
				break;
		}

		dataentry_head.offset += dataentry_head.boxsize;
	}
}
*/

void mp4_load::mp4_load_visitor::visit(BoxHead& head, TrackFragmentHeaderBox& tfhd)
{
	if ( !read_u32(tfhd.track_ID) ) {
		return;
	}

	if ( 0x000001 & head.flag ) {
		if ( !read_u64(tfhd.base_data_offset) ) {
			return;
		}
	}

	if ( 0x000002 & head.flag ) {
		if ( !read_u32(tfhd.sample_description_index) ) {
			return;
		}
	}

	if ( 0x000008 & head.flag ) {
		if ( !read_u32(tfhd.default_sample_duration) ) {
			return;
		}
	}

	if ( 0x000010 & head.flag ) {
		if ( !read_u32(tfhd.default_sample_size) ) {
			return;
		}
	}

	if ( 0x000020 & head.flag ) {
		if ( !read_u32(tfhd.default_sample_flags) ) {
			return;
		}
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, TrackRunBox& trun)
{
	uint32_t sample_count;
	if ( !read_u32(sample_count) ) {
		return;
	}

	if ( 0x000001 & head.flag ) {
		if ( !read_i32(trun.data_offset) ) {
			return;
		}
	}

	if ( 0x000004 & head.flag ) {
		if ( !read_u32(trun.first_sample_flags) ) {
			return;
		}
	}

	size_t samplesize = 0;
	if ( 0x000100 & head.flag ) samplesize += 4;
	if ( 0x000200 & head.flag ) samplesize += 4;
	if ( 0x000400 & head.flag ) samplesize += 4;
	if ( 0x000800 & head.flag ) samplesize += 4;

	if ( samplesize ) {
		for ( uint32_t i = 0; i < sample_count; i++ ) {
			uint8_t buf[samplesize];
			if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
				return;
			}

			TrackRunBox::Sample sample = { 0, 0, 0, 0 };
			uint8_t* bp = buf;

			if ( 0x000100 & head.flag ) {
				sample.sample_duration = U32(*bp, *(bp+1), *(bp+2), *(bp+3));
				bp += 4;
			}	

			if ( 0x000200 & head.flag ) {
				sample.sample_size = U32(*bp, *(bp+1), *(bp+2), *(bp+3));
				bp += 4;
			}

			if ( 0x000400 & head.flag ) {
				sample.sample_flags = U32(*bp, *(bp+1), *(bp+2), *(bp+3));
				bp += 4;
			}

			if ( 0x000800 & head.flag ) {
				sample.sample_composition_time_offset = U32(*bp, *(bp+1), *(bp+2), *(bp+3));
				bp += 4;
			}

			trun.samples.push_back(sample);
		}
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, SegmentIndexBox& sidx)
{
	uint32_t entry_count;
	if ( 1 == head.version ) {
		uint8_t buf[28];
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}

		sidx.reference_id = U32(buf[0], buf[1], buf[2], buf[3]);
		sidx.timescale = U32(buf[4], buf[5], buf[6], buf[7]);
		sidx.earliest_presentation_time = U64(buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
		sidx.first_offset = U64(buf[16], buf[17], buf[18], buf[19], buf[20], buf[21], buf[22], buf[23]);

		entry_count = U16(buf[26], buf[27]);
	}
	else { // version==0
		uint8_t buf[20];
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}

		sidx.reference_id = U32(buf[0], buf[1], buf[2], buf[3]);
		sidx.timescale = U32(buf[4], buf[5], buf[6], buf[7]);
		sidx.earliest_presentation_time = U32(buf[8], buf[9], buf[10], buf[11]);
		sidx.first_offset = U32(buf[12], buf[13], buf[14], buf[15]);

		entry_count = U16(buf[18], buf[19]);
	}

	for ( uint32_t i = 0; i < entry_count; i++ ) {
		uint8_t buf[12];
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}
		sidx.references.push_back( {
				(bool) ( buf[0] & 0x80 ),
				U32((uint32_t) ( buf[0] & 0x7F ), buf[1], buf[2], buf[3]),
				U32(buf[4], buf[5], buf[6], buf[7]),
				(bool) ( buf[8] & 0x80 ),
				U32((uint32_t) ( buf[8] & 0x7F ), buf[9], buf[10], buf[11])
				} );
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, MovieFragmentHeaderBox& mfhd)
{
	if ( !read_u32(mfhd.sequence_number) ) {
		return;
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, TrackFragmentDecodeTimeBox& tfdt)
{
	if ( !read_u64(tfdt.decode_time) ) {
		return;
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, TrackFragmentRandomAccessBox& tfra)
{
	if ( !read_u32(tfra.track_ID) ) {
		return;
	}

	uint8_t buf[4];
	if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
		return;
	}

	size_t length_size_of_traf_num = ( ( buf[3] & 0x30 ) >> 4 ) + 1;
	size_t length_size_of_trun_num = ( ( buf[3] & 0x0C ) >> 2 ) + 1;
	size_t length_size_of_sample_num = ( buf[3] & 0x03 ) + 1;

	uint32_t number_of_entry;
	if ( !read_u32(number_of_entry) ) {
		return;
	}

	switch ( head.version ) {
	case 1:
		{
			uint8_t entry_buf[16 + 12];
			for ( uint32_t i = 1; i <= number_of_entry; i++ ) {
				size_t entry_buf_size = 16 + length_size_of_traf_num + length_size_of_trun_num + length_size_of_sample_num;
				if ( _f->read(entry_buf, entry_buf_size) != entry_buf_size ) {
					return;
				}
				uint64_t time = U64(entry_buf[0], entry_buf[1], entry_buf[2], entry_buf[3], entry_buf[4], entry_buf[5], entry_buf[6], entry_buf[7]);
				uint64_t moof_offset = U64(entry_buf[8], entry_buf[9], entry_buf[10], entry_buf[11], entry_buf[12], entry_buf[13], entry_buf[14], entry_buf[15]);

				uint32_t traf_number = 0;
				uint32_t trun_number = 0;
				uint32_t sample_number = 0;
				const uint8_t* p = entry_buf + 16;

				for ( size_t traf_i = 0; traf_i < length_size_of_traf_num; traf_i++ ) {
					traf_number = traf_number * 0x100 + *p++;
				}

				for ( size_t trun_i = 0; trun_i < length_size_of_trun_num; trun_i++ ) {
					trun_number = trun_number * 0x100 + *p++;
				}

				for ( size_t sample_i = 0; sample_i < length_size_of_sample_num; sample_i++ ) {
					sample_number = sample_number * 0x100 + *p++;
				}

				tfra.entries[time] = {
					moof_offset,
					traf_number,
					trun_number,
					sample_number };
			}
			break;
		}

	default:
		{
			uint8_t entry_buf[8 + 12];
			for ( uint32_t i = 1; i <= number_of_entry; i++ ) {
				size_t entry_buf_size = 8 + length_size_of_traf_num + length_size_of_trun_num + length_size_of_sample_num;
				if ( _f->read(entry_buf, entry_buf_size) != entry_buf_size ) {
					return;
				}
				uint64_t time = U32(entry_buf[0], entry_buf[1], entry_buf[2], entry_buf[3]);
				uint64_t moof_offset = U32(entry_buf[4], entry_buf[5], entry_buf[6], entry_buf[7]);

				uint32_t traf_number = 0;
				uint32_t trun_number = 0;
				uint32_t sample_number = 0;
				const uint8_t* p = entry_buf + 8;

				for ( size_t traf_i = 0; traf_i < length_size_of_traf_num; traf_i++ ) {
					traf_number = traf_number * 0x100 + *p++;
				}

				for ( size_t trun_i = 0; trun_i < length_size_of_trun_num; trun_i++ ) {
					trun_number = trun_number * 0x100 + *p++;
				}

				for ( size_t sample_i = 0; sample_i < length_size_of_sample_num; sample_i++ ) {
					sample_number = sample_number * 0x100 + *p++;
				}

				tfra.entries[time] = {
					moof_offset,
					traf_number,
					trun_number,
					sample_number };
			}
		}
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, MovieFragmentRandomAccessOffsetBox& mfro)
{
	if ( !read_u32(mfro.size) ) {
		return;
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, TrackExtendsBox& trex)
{
	uint8_t buf[20];
	if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
		return;
	}
	trex.track_ID = U32(buf[0], buf[1], buf[2], buf[3]);
	trex.default_sample_description_index = U32(buf[4], buf[5], buf[6], buf[7]);
	trex.default_sample_duration = U32(buf[8], buf[9], buf[10], buf[11]);
	trex.default_sample_size = U32(buf[12], buf[13], buf[14], buf[15]);
	trex.default_sample_flags = U32(buf[16], buf[17], buf[18], buf[19]);
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, MediaDataBox& mdat)
{
	mdat.uri = _f->path();
	//mdat.chunks.push_back( {
	//	head.offset + head.boxheadsize,
	//	head.boxsize - head.boxheadsize
	//	} );
	//mdat.blocks[ head.offset + head.boxheadsize ] = head.boxsize - head.boxheadsize;
	mdat.byte_ranges[ head.offset + head.boxheadsize ] = head.boxsize - head.boxheadsize;
	mdat.chunks.clear();
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, DataBox<std::vector<uint8_t>>& box)
{
	size_t size = head.boxsize - head.boxheadsize;
	if ( 0 < size ) {
		uint8_t buf[size];
		if ( _f->read(buf, sizeof(buf)) != sizeof(buf) ) {
			return;
		}
		box.boxdata.assign(buf, buf + size);
	}
}

void mp4_load::mp4_load_visitor::visit(BoxHead& head, EmptyBox&)
{
}

// mp4_load

mp4_load::mp4_load(const char* uri)
	: _uri(uri)
{
}

mp4_load::~mp4_load()
{
}

void mp4_load::execute(std::shared_ptr<mp4_abstract_box> box)
{
	auto f = io::instance()->open( _uri );
	if ( f->is_open() ) {
		box->head().offset = 0;
		box->head().boxheadsize = 0;
		box->head().boxsize = f->size();

		mp4_load_visitor v(f);
		box->accept(&v);
		f->close();
	}
#ifdef _DEBUG
	else {
		fprintf(stderr, "** %s:%s(%d) failed to open %s\n", __func__, __FILE__, __LINE__, _uri.c_str());
	}
#endif
}

void mp4_load::execute(std::vector<std::shared_ptr<mp4_file>>& mp4files)
{
	std::shared_ptr<mp4_file> mp4 = std::make_shared<mp4_file>( _uri );
	execute(mp4);
	mp4files.push_back( mp4 );
}
