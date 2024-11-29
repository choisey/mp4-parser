/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_initialization_segment.h"
#include "mp4.h"
#include <assert.h>

// mp4_initialization_segment_visitor

mp4_initialization_segment::mp4_initialization_segment_visitor::mp4_initialization_segment_visitor(uint32_t track_id)
	: _track_id(track_id)
{
}

mp4_initialization_segment::mp4_initialization_segment_visitor::~mp4_initialization_segment_visitor()
{
}

void mp4_initialization_segment::mp4_initialization_segment_visitor::visit(BoxHead& head, std::vector<std::shared_ptr<mp4_abstract_box>>& boxes)
{
	switch ( head.boxtype ) {
		case MP4FILE:
			{
				std::vector<std::shared_ptr<mp4_abstract_box>> bb;

				for ( auto child: boxes ) {
					switch ( child->head().boxtype ) {
					case FREE:
					case SKIP:
					case MDAT:
						// Do not include
						break;

					default:
						bb.push_back(child);
						break;
					}
				}

				boxes = bb;
				break;
			}

		case MOOV:
			{
				// mehd
				std::shared_ptr<mp4_concrete_box<MovieExtendsHeaderBox>> mehd = std::make_shared<mp4_concrete_box<MovieExtendsHeaderBox>>(
						BoxHead { 0, 0, 0, MEHD, 0, 0 }
						);

				mehd->data().fragment_duration = 0;

				// trex
				std::shared_ptr<mp4_concrete_box<TrackExtendsBox>> trex = std::make_shared<mp4_concrete_box<TrackExtendsBox>>(
						BoxHead { 0, 0, 0, TREX, 0, 0 }
						);

				trex->data().track_ID = _track_id;
				trex->data().default_sample_description_index = 1;
				trex->data().default_sample_duration = 1;
				trex->data().default_sample_size = 0;
				trex->data().default_sample_flags = 0;

				// mvex
				std::shared_ptr<mp4_container_box> mvex = std::make_shared<mp4_container_box>(
						BoxHead { 0, 0, 0, MVEX, 0, 0 }
						);
				mvex->add_child(mehd);
				mvex->add_child(trex);

				std::vector<std::shared_ptr<mp4_abstract_box>> bb;

				for ( auto child: boxes ) {
					switch ( child->head().boxtype ) {
						case MVHD:
							{
								const auto& mvhd = select<MovieHeaderBox>(child);
								assert( 1 == mvhd.size() );
								if ( 1 == mvhd.size() ) {
									mehd->data().fragment_duration = mvhd[0]->data().duration;
								}

								bb.push_back(child);
								bb.push_back(mvex);
								break;
							}

						case TRAK:
							{
								const auto& tkhd = select<TrackHeaderBox>(child);
								assert( 1 == tkhd.size() );
								if ( 1 == tkhd.size() && tkhd[0]->data().track_ID == _track_id ) {

									const auto& hdlr = select<HandlerBox>(child);
									assert( 1 == hdlr.size() );
									if ( 1 == hdlr.size() && HandlerBox::VIDEO == hdlr[0]->data().handler_type ) {
										trex->data().default_sample_flags = 0x10000;
									}

									bb.push_back(child);
								}
								break;
							}

						default:
							bb.push_back(child);
							break;
					}
				}

				boxes = bb;
				break;
			}

		case TRAK:
			{
				std::vector<std::shared_ptr<mp4_abstract_box>> bb;

				for ( auto child: boxes ) {
					switch ( child->head().boxtype ) {
						case EDTS:
							// Do not include
							break;

						default:
							bb.push_back(child);
							break;
					}
				}

				boxes = bb;
				break;
			}

		case STBL:
			{
				std::vector<std::shared_ptr<mp4_abstract_box>> stbl_new_child_boxes;

				for ( auto child: boxes ) {
					switch ( child->head().boxtype ) {
						case STSS:
							// Do not include
							break;

						default:
							stbl_new_child_boxes.push_back(child);
							break;
					}
				}

				boxes = stbl_new_child_boxes;
				break;
			}
	}

	for ( auto child: boxes ) {
		child->accept(this);
	}
}

void mp4_initialization_segment::mp4_initialization_segment_visitor::visit(BoxHead& head, FileTypeBox& ftyp)
{
}

void mp4_initialization_segment::mp4_initialization_segment_visitor::visit(BoxHead& head, MovieHeaderBox& mvhd)
{
	mvhd.duration = 0;
}

void mp4_initialization_segment::mp4_initialization_segment_visitor::visit(BoxHead& head, TrackHeaderBox& tkhd)
{
	tkhd.duration = 0;
}

void mp4_initialization_segment::mp4_initialization_segment_visitor::visit(BoxHead& head, MediaHeaderBox& mdhd)
{
	mdhd.duration = 0;
}

void mp4_initialization_segment::mp4_initialization_segment_visitor::visit(BoxHead& head, TimeToSampleBox& stts)
{
	stts.entries.clear();
}

void mp4_initialization_segment::mp4_initialization_segment_visitor::visit(BoxHead& head, CompositionOffsetBox& ctts)
{
	ctts.entries.clear();
}

void mp4_initialization_segment::mp4_initialization_segment_visitor::visit(BoxHead& head, SampleToChunkBox& stsc)
{
	stsc.entries.clear();
}

void mp4_initialization_segment::mp4_initialization_segment_visitor::visit(BoxHead& head, SampleSizeBox& stsz)
{
	stsz.sample_size = 0;
	stsz.entry_sizes.clear();
}

void mp4_initialization_segment::mp4_initialization_segment_visitor::visit(BoxHead& head, ChunkOffsetBox& stco)
{
	stco.chunk_offsets.clear();
}

void mp4_initialization_segment::mp4_initialization_segment_visitor::visit(BoxHead& head, ChunkLargeOffsetBox& co64)
{
	co64.chunk_offsets.clear();
}

void mp4_initialization_segment::mp4_initialization_segment_visitor::visit(BoxHead& head, SyncSampleBox& stss)
{
}

void mp4_initialization_segment::mp4_initialization_segment_visitor::visit(BoxHead& head, MediaDataBox& mdat)
{
}

// mp4_initialization_segment

mp4_initialization_segment::mp4_initialization_segment(uint32_t track_id)
	: _track_id(track_id)
{
}

mp4_initialization_segment::~mp4_initialization_segment()
{
}

void mp4_initialization_segment::execute(std::vector<std::shared_ptr<mp4_file>>& mp4files)
{
	assert( mp4files.size() <= 1 );

	if ( !mp4files.empty() ) {
		mp4_initialization_segment_visitor v(_track_id);
		mp4files[0]->accept(&v);
	}
}
