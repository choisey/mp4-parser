/*
 * Copyright (c) Seungyeob Choi
 */

#include "MP4HlsM3u8.h"
#include "MP4.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define SLASH '/'

// MP4HlsM3u8

MP4HlsM3u8::MP4HlsM3u8(uint32_t duration)
	: MP4Playlist(duration)
{
}

MP4HlsM3u8::~MP4HlsM3u8()
{
}

void MP4HlsM3u8::execute(std::vector<std::shared_ptr<MP4File>>& mp4files)
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

	assert( mp4files.size() <= 1 );
	if ( 1 != mp4files.size() ) {
		return;
	}

	const uint32_t track_precedence[] = {
		HandlerBox::VIDEO,
		HandlerBox::AUDIO
	};

	for ( auto mp4: mp4files ) {
		size_t delim = mp4->path().find_last_of(SLASH);
		std::string file = url_encode( ( std::string::npos != delim )
				? mp4->path().substr( delim + 1 )
				: mp4->path() );

		for ( uint32_t handler_type: track_precedence ) {
			for ( auto trak: select(mp4, TRAK) ) {
				const auto& tkhd = select<TrackHeaderBox>(trak);
				assert( 1 == tkhd.size() );
				if ( tkhd.empty() ) continue;

				const auto& mdhd = select<MediaHeaderBox>(trak);
				assert( 1 == mdhd.size() );
				if ( mdhd.empty() ) {
					continue;
				}

				uint32_t timescale = mdhd[0]->data().timescale;

				const auto& hdlr = select<HandlerBox>(trak);
				assert( 1 == hdlr.size() );
				if ( hdlr.empty() ) continue;

				if ( hdlr[0]->data().handler_type != handler_type ) {
					// skip if the track is not the right type
					continue;
				}

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

				uint64_t time = 0;
				if ( !segments.empty() ) {
					uint64_t max_duration = 0;
					for ( auto segment: segments ) {
						if ( max_duration < segment.second ) {
							max_duration = segment.second;
						}
					}

					printf("#EXTM3U\n");
					printf("#EXT-X-VERSION:3\n");
					printf("#EXT-X-MEDIA-SEQUENCE:1\n");
					printf("#EXT-X-PLAYLIST-TYPE:VOD\n");
					printf("#EXT-X-ALLOW-CACHE:NO\n");
					printf("#EXT-X-TARGETDURATION:%lu\n", ( max_duration + ( timescale - 1 ) ) / timescale);

					for ( auto segment: segments ) {
						printf("#EXTINF:%0.3f,\n", (double) segment.second / timescale);
						printf("%s+%lu.ts\n", file.c_str(), time * 1000 / timescale);
						time += segment.second;
					}

					printf("#EXT-X-ENDLIST\n");
				}

				// after generating manifest using the first video track, return.
				return;
			}
		}
	}
}
