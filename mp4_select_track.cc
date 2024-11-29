/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_select_track.h"
#include "mp4.h"
#include <assert.h>
#include <string.h>

mp4_select_track::mp4_select_track_visitor::select_strategy::select_strategy()
{
}

mp4_select_track::mp4_select_track_visitor::select_strategy::~select_strategy()
{
}

mp4_select_track::mp4_select_track_visitor::select_by_number::select_by_number(uint32_t track_id)
	: _track_id(track_id)
{
	 _track_id = track_id;
}

mp4_select_track::mp4_select_track_visitor::select_by_number::~select_by_number()
{
}

bool mp4_select_track::mp4_select_track_visitor::select_by_number::is_selected(std::shared_ptr<mp4_abstract_box> trak)
{
	const auto& tkhd = select<TrackHeaderBox>(trak);
	assert( 1 == tkhd.size() );
	if ( 1 != tkhd.size() ) return false;
	return ( tkhd[0]->data().track_ID == _track_id ) ? true : false;
}

mp4_select_track::mp4_select_track_visitor::select_by_media::select_by_media(const char* media_type)
	: _media_type(media_type)
	, _selected(0)
{
}

mp4_select_track::mp4_select_track_visitor::select_by_media::~select_by_media()
{
}

bool mp4_select_track::mp4_select_track_visitor::select_by_media::is_selected(std::shared_ptr<mp4_abstract_box> trak)
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

// mp4_select_track_visitor

mp4_select_track::mp4_select_track_visitor::mp4_select_track_visitor(std::shared_ptr<mp4_select_track::mp4_select_track_visitor::select_strategy> selector)
	: _selector(selector)
{
}

mp4_select_track::mp4_select_track_visitor::~mp4_select_track_visitor()
{
}

void mp4_select_track::mp4_select_track_visitor::visit(BoxHead& head, std::vector<std::shared_ptr<mp4_abstract_box>>& boxes)
{
	switch ( head.boxtype ) {
		// moov
		case 0x6d6f6f76:
			{
				std::vector<std::shared_ptr<mp4_abstract_box>> new_child_boxes;
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

void mp4_select_track::mp4_select_track_visitor::visit(BoxHead& head, MovieHeaderBox& mvhd)
{
	mvhd.next_track_ID = 2;
}

void mp4_select_track::mp4_select_track_visitor::visit(BoxHead& head, TrackHeaderBox& tkhd)
{
	tkhd.track_ID = 1;
}

// mp4_select_track

mp4_select_track::mp4_select_track(uint32_t track_id)
{
	_selector = std::make_shared<mp4_select_track::mp4_select_track_visitor::select_by_number>(track_id);
}

mp4_select_track::mp4_select_track(const char* media_type)
{
	_selector = std::make_shared<mp4_select_track::mp4_select_track_visitor::select_by_media>(media_type);
}

mp4_select_track::~mp4_select_track()
{
}

void mp4_select_track::execute(std::shared_ptr<mp4_abstract_box> mp4)
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

	mp4_select_track_visitor v(_selector);
	mp4->accept(&v);
}
