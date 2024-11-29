/*
 * Copyright (c) Seungyeob Choi
 */

#include "MP4SelectTrack.h"
#include "mp4.h"
#include <assert.h>
#include <string.h>

MP4SelectTrack::MP4SelectTrackVisitor::SelectStrategy::SelectStrategy()
{
}

MP4SelectTrack::MP4SelectTrackVisitor::SelectStrategy::~SelectStrategy()
{
}

MP4SelectTrack::MP4SelectTrackVisitor::SelectByNumber::SelectByNumber(uint32_t track_id)
	: _track_id(track_id)
{
	 _track_id = track_id;
}

MP4SelectTrack::MP4SelectTrackVisitor::SelectByNumber::~SelectByNumber()
{
}

bool MP4SelectTrack::MP4SelectTrackVisitor::SelectByNumber::is_selected(std::shared_ptr<MP4AbstractBox> trak)
{
	const auto& tkhd = select<TrackHeaderBox>(trak);
	assert( 1 == tkhd.size() );
	if ( 1 != tkhd.size() ) return false;
	return ( tkhd[0]->data().track_ID == _track_id ) ? true : false;
}

MP4SelectTrack::MP4SelectTrackVisitor::SelectByMedia::SelectByMedia(const char* media_type)
	: _media_type(media_type)
	, _selected(0)
{
}

MP4SelectTrack::MP4SelectTrackVisitor::SelectByMedia::~SelectByMedia()
{
}

bool MP4SelectTrack::MP4SelectTrackVisitor::SelectByMedia::is_selected(std::shared_ptr<MP4AbstractBox> trak)
{
	const auto& hdlr = select<HandlerBox>(trak);
	assert( 1 == hdlr.size() );
	if ( 1 != hdlr.size() ) return false;
	if ( 0 == strncmp(_media_type.c_str(), hdlr[0]->data().handler_type, 4) ) {
		if ( 1 == ++_selected ) {
			return true;
		}
	}
	return false;
}

// MP4SelectTrackVisitor

MP4SelectTrack::MP4SelectTrackVisitor::MP4SelectTrackVisitor(std::shared_ptr<MP4SelectTrack::MP4SelectTrackVisitor::SelectStrategy> selector)
	: _selector(selector)
{
}

MP4SelectTrack::MP4SelectTrackVisitor::~MP4SelectTrackVisitor()
{
}

void MP4SelectTrack::MP4SelectTrackVisitor::visit(BoxHead& head, std::vector<std::shared_ptr<MP4AbstractBox>>& boxes)
{
	switch ( head.boxtype ) {
		// moov
		case 0x6d6f6f76:
			{
				std::vector<std::shared_ptr<MP4AbstractBox>> new_child_boxes;
				for ( auto child: boxes ) {
					switch ( child->head().boxtype ) {
						// trak
						case 0x7472616b:
							if ( !_selector->is_selected(child) ) {
								continue;
							}
					}

					new_child_boxes.push_back(child);
				}
				boxes = new_child_boxes;
			}
	}

	for ( auto child: boxes ) {
		child->accept(this);
	}
}

void MP4SelectTrack::MP4SelectTrackVisitor::visit(BoxHead& head, MovieHeaderBox& mvhd)
{
	mvhd.next_track_ID = 2;
}

void MP4SelectTrack::MP4SelectTrackVisitor::visit(BoxHead& head, TrackHeaderBox& tkhd)
{
	tkhd.track_ID = 1;
}

// MP4SelectTrack

MP4SelectTrack::MP4SelectTrack(uint32_t track_id)
{
	_selector = std::make_shared<MP4SelectTrack::MP4SelectTrackVisitor::SelectByNumber>(track_id);
}

MP4SelectTrack::MP4SelectTrack(const char* media_type)
{
	_selector = std::make_shared<MP4SelectTrack::MP4SelectTrackVisitor::SelectByMedia>(media_type);
}

MP4SelectTrack::~MP4SelectTrack()
{
}

void MP4SelectTrack::execute(std::shared_ptr<MP4AbstractBox> mp4)
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
	//			          |   +------+
	//			          +-- | MDIA |
	//			              +------+

	MP4SelectTrackVisitor v(_selector);
	mp4->accept(&v);
}
