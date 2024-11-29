/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_media_stat.h"
#include "mp4.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <typeinfo>

// mp4_media_stat

mp4_media_stat::mp4_media_stat()
{
}

mp4_media_stat::~mp4_media_stat()
{
}

void mp4_media_stat::execute(std::shared_ptr<mp4_abstract_box> mp4)
{
	std::vector<Track> tracks;
	for ( auto trak: select(mp4, TRAK) ) {
		const auto& tkhd = select<TrackHeaderBox>(trak);
		assert( 1 == tkhd.size() );
		if ( 1 != tkhd.size() ) continue;

		for ( auto mdia: select(trak, MDIA) ) {
			const auto& hdlr = select<HandlerBox>(mdia);
			assert( 1 == hdlr.size() );
			if ( 1 != hdlr.size() ) break;

			const auto& mdhd = select<MediaHeaderBox>(mdia);
			assert( 1 == mdhd.size() );
			if ( 1 != mdhd.size() ) break;

			const auto& stsz = select<SampleSizeBox>(mdia);
			assert( 1 == stsz.size() );
			if ( 1 != stsz.size() ) break;

			uint64_t size = 0;
			for ( auto sz: stsz[0]->data().entry_sizes ) {
				size += sz;
			}

			Track t;
			t.id = tkhd[0]->data().track_ID;
			t.handler_type = hdlr[0]->data().handler_type;
			t.duration = (double) mdhd[0]->data().duration / mdhd[0]->data().timescale;
			t.size = size;

			tracks.push_back(t);
		}
	}

	for ( const auto& t: tracks ) {
		printf("id=%u, handler_type=%c%c%c%c, size=%lu, duration=%f, bitrate=%u\n", t.id,
			(char) ( t.handler_type / 0x1000000 ),
			(char) ( ( t.handler_type / 0x10000 ) & 0xff ),
			(char) ( ( t.handler_type / 0x100 ) & 0xff ),
			(char) ( t.handler_type & 0xff ),
			t.size, t.duration, (uint32_t) ( t.size * 8 / t.duration ));
	}
}
