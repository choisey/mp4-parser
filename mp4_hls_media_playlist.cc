/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_hls_media_playlist.h"
#include "mp4.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// MP4HlsMediaPlaylist

MP4HlsMediaPlaylist::MP4HlsMediaPlaylist(uint32_t track_id, uint32_t duration)
	: _track_id(track_id), MP4Playlist(duration)
{
}

MP4HlsMediaPlaylist::~MP4HlsMediaPlaylist()
{
}

void MP4HlsMediaPlaylist::execute(std::shared_ptr<mp4_abstract_box> mp4)
{
	/*
		+------+       +------+
		| TRAK | --+-- | TKHD |
		+------+   |   +------+
		           |
		           |   +------+       +------+
		           +-- | MDIA | --+-- | MDHD |
		               +------+   |   +------+
		                          |
		                          |   +------+
		                          +-- | HDLR |
		                          |   +------+
		                          |
		                          |   +------+       +------+       +------+
		                          +-- | MINF | --+-- | STBL | --+-- | STSS |
		                              +------+       +------+       +------+
	*/

	const auto& mvhd = select<MovieHeaderBox>(mp4);
	assert( 1 == mvhd.size() );
	if ( mvhd.empty() ) {
		return;
	}

	for ( auto trak: select(mp4, TRAK) ) {
		const auto& tkhd = select<TrackHeaderBox>(trak);
		assert( 1 == tkhd.size() );
		if ( tkhd.empty() ) continue;
		if ( tkhd[0]->data().track_ID != _track_id ) continue;

		const auto& mdhd = select<MediaHeaderBox>(trak);
		assert( 1 == mdhd.size() );
		if ( mdhd.empty() ) {
			continue;
		}

		uint32_t timescale = mdhd[0]->data().timescale;

		const auto& hdlr = select<HandlerBox>(trak);
		assert( 1 == hdlr.size() );
		if ( hdlr.empty() ) continue;

		const auto& mdia = select(trak, MDIA);
		assert( 1 == mdia.size() );
		if ( mdia.empty() ) continue;

		const auto& minf = select(mdia[0], MINF);
		assert( 1 == minf.size() );
		if ( minf.empty() ) continue;

		std::vector<std::pair<uint64_t, uint64_t>> segments;

		minf_to_segments(
				minf[0], timescale,
				segments);

#ifdef _DEBUG
		std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ":segments (start, duration in millisecond)=";
		if ( !segments.empty() ) {
			std::vector<std::pair<uint64_t, uint64_t>> segmentdump;
			for ( auto segment: segments ) {
				segmentdump.push_back(std::make_pair(
							segment.first * 1000 / timescale,
							segment.second * 1000 / timescale));
			}
			std::cerr << segmentdump;
		}
		std::cerr << std::endl;
#endif

		if ( !segments.empty() ) {
			uint64_t max_duration = 0;
			for ( auto segment: segments ) {
				if ( max_duration < segment.second ) {
					max_duration = segment.second;
				}
			}

			printf("#EXTM3U\n");
			printf("#EXT-X-VERSION:7\n");
			printf("#EXT-X-MEDIA-SEQUENCE:1\n");
			printf("#EXT-X-PLAYLIST-TYPE:VOD\n");
			printf("#EXT-X-INDEPENDENT-SEGMENTS\n");
			printf("#EXT-X-MAP:URI=\"%u.mp4\"\n", tkhd[0]->data().track_ID);
			printf("#EXT-X-TARGETDURATION:%lu\n", ( max_duration + ( timescale - 1 ) ) / timescale);

			uint64_t time = 0;
			for ( auto segment: segments ) {
				printf("#EXTINF:%0.5f,\n", (double) segment.second / timescale);
				printf("%u+%lu.mp4\n", tkhd[0]->data().track_ID, time);
				time += segment.second;
			}

			printf("#EXT-X-ENDLIST\n");
		}

		return;
	}
}

void MP4HlsMediaPlaylist::execute(std::vector<std::shared_ptr<MP4File>>& mp4files)
{
	assert( mp4files.size() <= 1 );

	if ( !mp4files.empty() ) {
		execute(mp4files[0]);
	}
}
