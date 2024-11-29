/*
 * Copyright (c) Seungyeob Choi
 */

#include "MP4DashMpd.h"
#include "mp4.h"
#include <assert.h>
#include <stdio.h>

#define INITIALIZATION_EXT ".mp4"
#define FRAGMENT_EXT ".mp4"

// SampleEntry is 8 bytes
// VisualSampleEntry is SampleEntry + 70 bytes
//#define STSD_VISUALSAMPLEENTRY_SIZE	( 8 + 70 )

#define SLASH '/'
#define MAX_FILENAME 260

// MP4DashMpd

MP4DashMpd::MP4DashMpd(uint32_t duration)
	: MP4Playlist(duration)
{
}

MP4DashMpd::~MP4DashMpd()
{
}

void MP4DashMpd::execute(std::vector<std::shared_ptr<MP4File>>& mp4files)
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

	struct TrackInfo {
		std::string name;
		uint32_t track_id;
		uint32_t timescale;
		double duration;
		std::string codecs;
		uint32_t width;
		uint32_t height;
		float frame_rate;
		std::vector<std::pair<uint64_t, uint64_t>> segments;
	};

	std::map<size_t, TrackInfo> video_tracks;
	std::map<size_t, TrackInfo> audio_tracks;

	for ( auto mp4: mp4files ) {
		size_t delim = mp4->path().find_last_of(SLASH);
		std::string file = url_encode( ( std::string::npos != delim )
				? mp4->path().substr( delim + 1 )
				: mp4->path() );

		for ( auto trak: select(mp4, TRAK) ) {
			const auto& tkhd = select<TrackHeaderBox>(trak);
			assert( 1 == tkhd.size() );
			if ( tkhd.empty() ) continue;

			const auto& mdhd = select<MediaHeaderBox>(trak);
			assert( 1 == mdhd.size() );
			if ( mdhd.empty() ) {
				continue;
			}

			const auto& hdlr = select<HandlerBox>(trak);
			assert( 1 == hdlr.size() );
			if ( hdlr.empty() ) continue;

			if ( hdlr[0]->data().handler_type != HandlerBox::VIDEO ) {
				// skip if the track is not the right type
				continue;
			}

			const auto& mdia = select(trak, MDIA);
			assert( 1 == mdia.size() );
			if ( mdia.empty() ) continue;

			const auto& minf = select(mdia[0], MINF);
			assert( 1 == minf.size() );
			if ( minf.empty() ) continue;

			const auto& stts = select<TimeToSampleBox>(minf[0]);
			assert( 1 == stts.size() );
			assert( 1 <= stts[0]->data().entries.size() );
			if ( stts.empty() || stts[0]->data().entries.empty() ) continue;

			const auto& stsz = select<SampleSizeBox>(minf[0]);
			assert( 1 == stsz.size() );
			if ( stsz.empty() ) continue;

			uint64_t size = 0;
			for ( auto sz: stsz[0]->data().entry_sizes ) {
				size += sz;
			}

			if ( video_tracks.end() == video_tracks.find(size) ) {
				std::string codecs;
				if ( !decoder_conf_string(minf[0], codecs) ) {
					// video codec is not AVC
					continue;
				}
				char buf[MAX_FILENAME];
				snprintf(buf, sizeof(buf), "%s/%u", file.c_str(), tkhd[0]->data().track_ID);
				auto ret = video_tracks.insert( std::pair<size_t, TrackInfo>( size, TrackInfo{
					buf, tkhd[0]->data().track_ID,
					mdhd[0]->data().timescale, (double) mdhd[0]->data().duration / mdhd[0]->data().timescale,
					codecs,
					tkhd[0]->data().width / 0x10000,
					tkhd[0]->data().height / 0x10000,
					(float) mdhd[0]->data().timescale / stts[0]->data().entries[0].sample_delta, {} } ) );
				assert( true == ret.second );
				if ( true == ret.second ) {
					minf_to_segments(
							minf[0], mdhd[0]->data().timescale,
							ret.first->second.segments);
				}
			}
		}

		for ( auto trak: select(mp4, TRAK) ) {
			const auto& tkhd = select<TrackHeaderBox>(trak);
			assert( 1 == tkhd.size() );
			if ( tkhd.empty() ) continue;

			const auto& mdhd = select<MediaHeaderBox>(trak);
			assert( 1 == mdhd.size() );
			if ( mdhd.empty() ) {
				continue;
			}

			const auto& hdlr = select<HandlerBox>(trak);
			assert( 1 == hdlr.size() );
			if ( hdlr.empty() ) continue;

			if ( hdlr[0]->data().handler_type != HandlerBox::AUDIO ) {
				// skip if the track is not the right type
				continue;
			}

			const auto& mdia = select(trak, MDIA);
			assert( 1 == mdia.size() );
			if ( mdia.empty() ) continue;

			const auto& minf = select(mdia[0], MINF);
			assert( 1 == minf.size() );
			if ( minf.empty() ) continue;

			const auto& stsz = select<SampleSizeBox>(minf[0]);
			assert( 1 == stsz.size() );
			if ( stsz.empty() ) continue;

			uint64_t size = 0;
			for ( auto sz: stsz[0]->data().entry_sizes ) {
				size += sz;
			}

			if ( audio_tracks.end() == audio_tracks.find(size) ) {
				std::string codecs;
				if ( !decoder_conf_string(minf[0], codecs) ) {
					continue;
				}
				char buf[MAX_FILENAME];
				snprintf(buf, sizeof(buf), "%s/%u", file.c_str(), tkhd[0]->data().track_ID);
				auto ret = audio_tracks.insert( std::pair<size_t, TrackInfo>( size, TrackInfo{
					buf, tkhd[0]->data().track_ID,
					mdhd[0]->data().timescale, (double) mdhd[0]->data().duration / mdhd[0]->data().timescale,
					codecs,
					0, 0,
					0, {} } ) );
				assert( true == ret.second );
				if ( true == ret.second ) {
					minf_to_segments(
							minf[0], mdhd[0]->data().timescale,
							ret.first->second.segments);
				}
			}
		}
	}

	if ( !video_tracks.empty() || !audio_tracks.empty() ) {
		const auto& mvhd = select<MovieHeaderBox>(mp4files[0]);
		assert( 1 == mvhd.size() );
		if ( mvhd.empty() ) {
			return;
		}

		uint64_t duration = mvhd[0]->data().duration / mvhd[0]->data().timescale;
		uint32_t duration_h = duration / 3600;
		uint32_t duration_m = duration / 60 % 60;
		double duration_s = (double) ( ( mvhd[0]->data().duration * 1000 / mvhd[0]->data().timescale ) % 60000 ) / 1000;

		printf("<?xml version=\"1.0\"?>\n");
		printf("<MPD \
type=\"static\" \
xmlns=\"urn:mpeg:dash:schema:mpd:2011\" \
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \
xsi:schemaLocation=\"urn:mpeg:DASH:schema:MPD:2011 DASH-MPD.xsd\" \
profiles=\"urn:mpeg:dash:profile:isoff-on-demand:2011\" \
minBufferTime=\"PT1.5S\" \
mediaPresentationDuration=\"PT%uH%02uM%02.3fS\">\n",
				duration_h,
				duration_m,
				duration_s);
		printf("  <Period start=\"PT0S\" duration=\"PT%uH%02uM%02.3fS\">\n",
				duration_h,
				duration_m,
				duration_s);

		uint32_t group_cnt = 0;

		if ( !video_tracks.empty() ) {
			TrackInfo& v1 = video_tracks.begin()->second;
			printf("    <AdaptationSet group=\"%u\" contentType=\"video\" mimeType=\"video/mp4\" segmentAlignment=\"true\" startWithSAP=\"1\">\n", ++group_cnt);
			printf("      <SegmentTemplate timescale=\"%u\" initialization=\"$RepresentationID$" INITIALIZATION_EXT "\" media=\"$RepresentationID$+$Time$" FRAGMENT_EXT "\">\n", v1.timescale);
			printf("        <SegmentTimeline>\n");

			int segment_count = 0;
			printf("          ");
			for ( auto segment: v1.segments ) {
				if ( 0 == segment_count++ ) {
					printf("<S t=\"0\" d=\"%lu\"/>", segment.second);
				}
				else {
					printf("<S d=\"%lu\"/>", segment.second);
				}
			}
			printf("\n");
			printf("        </SegmentTimeline>\n");
			printf("      </SegmentTemplate>\n");
			for ( auto v: video_tracks ) {
				printf("      <Representation id=\"%s\" codecs=\"%s\" width=\"%u\" height=\"%u\" bandwidth=\"%lu\" />\n",
						v.second.name.c_str(),
						v.second.codecs.c_str(),
						v.second.width, v.second.height,
						(uint64_t) ( (double) ( v.first * 8 ) / v.second.duration ) );
			}
			printf("    </AdaptationSet>\n");
		}

		if ( !audio_tracks.empty() ) {
			TrackInfo& a1 = audio_tracks.begin()->second;
			printf("    <AdaptationSet group=\"%u\" contentType=\"audio\" mimeType=\"audio/mp4\" segmentAlignment=\"true\" startWithSAP=\"1\">\n", ++group_cnt);
			printf("      <SegmentTemplate timescale=\"%u\" initialization=\"$RepresentationID$" INITIALIZATION_EXT "\" media=\"$RepresentationID$+$Time$" FRAGMENT_EXT "\">\n", a1.timescale);
			printf("        <SegmentTimeline>\n");

			int segment_count = 0;
			printf("          ");
			for ( auto segment: a1.segments ) {
				if ( 0 == segment_count++ ) {
					printf("<S t=\"0\" d=\"%lu\"/>", segment.second);
				}
				else {
					printf("<S d=\"%lu\"/>", segment.second);
				}
			}
			printf("\n");
			printf("        </SegmentTimeline>\n");
			printf("      </SegmentTemplate>\n");
			for ( auto a: audio_tracks ) {
				printf("      <Representation id=\"%s\" codecs=\"%s\" bandwidth=\"%lu\" />\n",
						a.second.name.c_str(),
						a.second.codecs.c_str(),
						(uint64_t) ( (double) ( a.first * 8 ) / a.second.duration ) );
			}
			printf("    </AdaptationSet>\n");
		}

		printf("  </Period>\n");
		printf("</MPD>\n");
	}
}
