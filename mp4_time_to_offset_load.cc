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

// MP4TimeToOffsetLoadVisitor

MP4TimeToOffsetLoad::MP4TimeToOffsetLoadVisitor::MP4TimeToOffsetLoadVisitor(std::shared_ptr<io_file> f)
	: mp4_loadVisitor(f)
{
}

MP4TimeToOffsetLoad::MP4TimeToOffsetLoadVisitor::~MP4TimeToOffsetLoadVisitor()
{
}

void MP4TimeToOffsetLoad::MP4TimeToOffsetLoadVisitor::visit(BoxHead& head, std::vector<std::shared_ptr<mp4_abstract_box>>& boxes)
{
	assert( _f->is_open() );

	if ( MP4FILE == head.boxtype ) {
		assert( 0 == head.offset );

		// MFRO: go to the end of the file
		if ( !_f->seek( MFRO_OFFSET, SEEK_END ) ) {
			return;
		}

		BoxHead mfro_head;

		if ( !readBoxHead(mfro_head) || MFRO != mfro_head.boxtype ) {
			return;
		}

		auto mfro_box = newBox<mp4_concrete_box<MovieFragmentRandomAccessOffsetBox>>(mfro_head);
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

		while ( ( nb = readBox(offset, head.boxtype, boxes) ) != 0 ) {
			offset += nb;
			_f->seek(offset, SEEK_SET);
		}
	}
	else {
		assert( 0 != head.offset );
		mp4_loadVisitor::visit(head, boxes);
	}
}

// MP4TimeToOffsetLoad

MP4TimeToOffsetLoad::MP4TimeToOffsetLoad(const char* uri)
	: mp4_load(uri)
{
}

MP4TimeToOffsetLoad::~MP4TimeToOffsetLoad()
{
}

void MP4TimeToOffsetLoad::execute(std::shared_ptr<mp4_abstract_box> box)
{
	auto f = io::instance()->open( _uri );
	if ( f->is_open() ) {
		box->head().offset = 0;
		box->head().boxheadsize = 0;
		box->head().boxsize = f->size();

		MP4TimeToOffsetLoadVisitor v(f);
		box->accept(&v);
		f->close();
	}
#ifdef _DEBUG
	else {
		fprintf(stderr, "** %s:%s(%d) failed to open %s\n", __func__, __FILE__, __LINE__, _uri.c_str());
	}
#endif
}

void MP4TimeToOffsetLoad::execute(std::vector<std::shared_ptr<MP4File>>& mp4files)
{
	std::shared_ptr<MP4File> mp4 = std::make_shared<MP4File>( _uri );
	execute(mp4);
	mp4files.push_back( mp4 );
}
