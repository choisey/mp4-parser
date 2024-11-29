/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_frag_key_frame.h"
//#include "mp4.h"
#include <assert.h>

// mp4_frag_key_frame_visitor

mp4_frag_key_frame::mp4_frag_key_frame_visitor::mp4_frag_key_frame_visitor()
	: _mdat_size(0)
{
}

mp4_frag_key_frame::mp4_frag_key_frame_visitor::~mp4_frag_key_frame_visitor()
{
}

void mp4_frag_key_frame::mp4_frag_key_frame_visitor::visit(BoxHead& head, TrackRunBox& trun)
{
	if ( 1 < trun.samples.size() ) {
		trun.samples.resize(1);
		_mdat_size = trun.samples.front().sample_size;
	}
}

void mp4_frag_key_frame::mp4_frag_key_frame_visitor::visit(BoxHead& head, MediaDataBox& mdat)
{
	if ( 0 != _mdat_size ) {
		mdat.byte_ranges.clear();
		mdat.chunks.clear();

		mdat.byte_ranges[ head.offset + head.boxheadsize ] = _mdat_size;
	}
}

// mp4_frag_key_frame

mp4_frag_key_frame::mp4_frag_key_frame()
{
}

mp4_frag_key_frame::~mp4_frag_key_frame()
{
}

void mp4_frag_key_frame::execute(std::shared_ptr<mp4_abstract_box> mp4)
{
	assert( MP4FILE == mp4->head().boxtype );

	mp4_frag_key_frame_visitor v;
	mp4->accept(&v);
}
