/*
 * Copyright (c) Seungyeob Choi
 */

// MP4 parser
// based on ISO/IEC 14496-12:2005(E)

#include "mp4_time_to_offset_load.h"
#include "mp4.h"
#include "io.h"
#include <assert.h>

#define MFRO_OFFSET	-16

// mp4_time_to_offset_load_visitor

mp4_time_to_offset_load::mp4_time_to_offset_load_visitor::mp4_time_to_offset_load_visitor(std::shared_ptr<io_file> f)
	: mp4_load_visitor(f)
{
}

mp4_time_to_offset_load::mp4_time_to_offset_load_visitor::~mp4_time_to_offset_load_visitor()
{
}

void mp4_time_to_offset_load::mp4_time_to_offset_load_visitor::visit(BoxHead& head, std::vector<std::shared_ptr<mp4_abstract_box>>& boxes)
{
	assert( _f->is_open() );

	if ( MP4FILE == head.boxtype ) {
		assert( 0 == head.offset );

		// MFRO: go to the end of the file
		if ( !_f->seek( MFRO_OFFSET, SEEK_END ) ) {
			return;
		}

		BoxHead mfro_head;

		if ( !read_box_head(mfro_head) || MFRO != mfro_head.boxtype ) {
			return;
		}

		auto mfro_box = new_box<mp4_concrete_box<MovieFragmentRandomAccessOffsetBox>>(mfro_head);
		mfro_box->accept(this);

		// MFRA
		if ( !_f->seek( -( (long int) mfro_box->data().size ), SEEK_END ) ) {
			return;
		}

		long int pos = _f->position();
		if ( pos < 0 ) {
			return;
		}

		size_t offset = pos;
		size_t nb;

		while ( ( nb = read_box(offset, head.boxtype, boxes) ) != 0 ) {
			offset += nb;
			_f->seek(offset, SEEK_SET);
		}
	}
	else {
		assert( 0 != head.offset );
		mp4_load_visitor::visit(head, boxes);
	}
}

// mp4_time_to_offset_load

mp4_time_to_offset_load::mp4_time_to_offset_load(const char* uri)
	: mp4_load(uri)
{
}

mp4_time_to_offset_load::~mp4_time_to_offset_load()
{
}

void mp4_time_to_offset_load::execute(std::shared_ptr<mp4_abstract_box> box)
{
	auto f = io::instance()->open( _uri );
	if ( f->is_open() ) {
		box->head().offset = 0;
		box->head().boxheadsize = 0;
		box->head().boxsize = f->size();

		mp4_time_to_offset_load_visitor v(f);
		box->accept(&v);
		f->close();
	}
#ifdef _DEBUG
	else {
		fprintf(stderr, "** %s:%s(%d) failed to open %s\n", __func__, __FILE__, __LINE__, _uri.c_str());
	}
#endif
}

void mp4_time_to_offset_load::execute(std::vector<std::shared_ptr<mp4_file>>& mp4files)
{
	std::shared_ptr<mp4_file> mp4 = std::make_shared<mp4_file>( _uri );
	execute(mp4);
	mp4files.push_back( mp4 );
}
