/*
 * Copyright (c) Seungyeob Choi
 */

#include "MP4InitializationSegment.h"
#include "mp4.h"
#include <assert.h>

// MP4InitializationSegmentVisitor

MP4InitializationSegment::MP4InitializationSegmentVisitor::MP4InitializationSegmentVisitor(uint32_t track_id)
	: _track_id(track_id)
{
}

MP4InitializationSegment::MP4InitializationSegmentVisitor::~MP4InitializationSegmentVisitor()
{
}

void MP4InitializationSegment::MP4InitializationSegmentVisitor::visit(BoxHead& head, std::vector<std::shared_ptr<mp4_abstract_box>>& boxes)
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
				mvex->addChild(mehd);
				mvex->addChild(trex);

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

void MP4InitializationSegment::MP4InitializationSegmentVisitor::visit(BoxHead& head, FileTypeBox& ftyp)
{
}

void MP4InitializationSegment::MP4InitializationSegmentVisitor::visit(BoxHead& head, MovieHeaderBox& mvhd)
{
	mvhd.duration = 0;
}

void MP4InitializationSegment::MP4InitializationSegmentVisitor::visit(BoxHead& head, TrackHeaderBox& tkhd)
{
	tkhd.duration = 0;
}

void MP4InitializationSegment::MP4InitializationSegmentVisitor::visit(BoxHead& head, MediaHeaderBox& mdhd)
{
	mdhd.duration = 0;
}

void MP4InitializationSegment::MP4InitializationSegmentVisitor::visit(BoxHead& head, TimeToSampleBox& stts)
{
	stts.entries.clear();
}

void MP4InitializationSegment::MP4InitializationSegmentVisitor::visit(BoxHead& head, CompositionOffsetBox& ctts)
{
	ctts.entries.clear();
}

void MP4InitializationSegment::MP4InitializationSegmentVisitor::visit(BoxHead& head, SampleToChunkBox& stsc)
{
	stsc.entries.clear();
}

void MP4InitializationSegment::MP4InitializationSegmentVisitor::visit(BoxHead& head, SampleSizeBox& stsz)
{
	stsz.sample_size = 0;
	stsz.entry_sizes.clear();
}

void MP4InitializationSegment::MP4InitializationSegmentVisitor::visit(BoxHead& head, ChunkOffsetBox& stco)
{
	stco.chunk_offsets.clear();
}

void MP4InitializationSegment::MP4InitializationSegmentVisitor::visit(BoxHead& head, ChunkLargeOffsetBox& co64)
{
	co64.chunk_offsets.clear();
}

void MP4InitializationSegment::MP4InitializationSegmentVisitor::visit(BoxHead& head, SyncSampleBox& stss)
{
}

void MP4InitializationSegment::MP4InitializationSegmentVisitor::visit(BoxHead& head, MediaDataBox& mdat)
{
}

// MP4InitializationSegment

MP4InitializationSegment::MP4InitializationSegment(uint32_t track_id)
	: _track_id(track_id)
{
}

MP4InitializationSegment::~MP4InitializationSegment()
{
}

void MP4InitializationSegment::execute(std::vector<std::shared_ptr<MP4File>>& mp4files)
{
	assert( mp4files.size() <= 1 );

	if ( !mp4files.empty() ) {
		MP4InitializationSegmentVisitor v(_track_id);
		mp4files[0]->accept(&v);
	}
}
