/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_hls_master_playlist.h"
#include "mp4.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define SLASH '/'
#define MAX_FILENAME 260

// mp4_hls_master_playlist

mp4_hls_master_playlist::mp4_hls_master_playlist(uint32_t v) : _version(v)
{
}

mp4_hls_master_playlist::~mp4_hls_master_playlist()
{
}

void mp4_hls_master_playlist::langCodeToName(
		uint8_t cc1, uint8_t cc2, uint8_t cc3,
		std::string& lang2,
		std::string& langName)
{
	langCodeToName(
			((unsigned int) cc1) * 0x10000
			+ ((unsigned int) cc2) * 0x100
			+ (unsigned int) cc3,
			lang2, langName);
}

void mp4_hls_master_playlist::langCodeToName(
		uint32_t langCode,
		std::string& lang2,
		std::string& langName)
{
	// https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
	switch ( langCode ) {
		case 0x617261: // ara
			langName = "Arabic";
			lang2 = "ar";
			break;

		case 0x636573: // ces
		case 0x637a65: // cze
			langName = "Czech";
			lang2 = "cs";
			break;

		case 0x636869: // chi
		case 0x7a686f: // zho
			langName = "Chinese";
			lang2 = "zh";
			break;

		case 0x64616e: // dan
			langName = "Danish";
			lang2 = "da";
			break;

		case 0x646575: // deu
		case 0x676572: // ger
			langName = "German";
			lang2 = "de";
			break;

		case 0x656c6c: // ell
		case 0x677265: // gre
			langName = "Greek";
			lang2 = "el";
			break;

		case 0x656e67: // eng
			langName = "English";
			lang2 = "en";
			break;

		case 0x666173: // fas
		case 0x706572: // per
			langName = "Persian";
			lang2 = "fa";
			break;

		case 0x66696e: // fin
			langName = "Finnish";
			lang2 = "fi";
			break;

		case 0x667261: // fra
		case 0x667265: // fre
			langName = "French";
			lang2 = "fr";
			break;

		case 0x68696e: // hin
			langName = "Hindi";
			lang2 = "hi";
			break;

		case 0x68756e: // hun
			langName = "Hungarian";
			lang2 = "hu";
			break;

		case 0x696e64: // ind
			langName = "Indonesian";
			lang2 = "id";
			break;

		case 0x697461: // ita
			langName = "Italian";
			lang2 = "it";
			break;

		case 0x6a706e: // jpn
			langName = "Japanese";
			lang2 = "ja";
			break;

		case 0x6b6f72: // kor
			langName = "Korean";
			lang2 = "jo";
			break;

		case 0x6e6f72: // nor
			langName = "Norwegian";
			lang2 = "no";
			break;

		case 0x706f6c: // pol
			langName = "Polish";
			lang2 = "pl";
			break;

		case 0x706f72: // por
			langName = "Portuguese";
			lang2 = "pt";
			break;

		case 0x727573: // rus
			langName = "Russian";
			lang2 = "ru";
			break;

		case 0x737061: // spa
			langName = "Spanish";
			lang2 = "es";
			break;

		case 0x737765: // swe
			langName = "Swedish";
			lang2 = "sv";
			break;

		case 0x746861: // tha
			langName = "Thai";
			lang2 = "th";
			break;

		case 0x747572: // tur
			langName = "Turkish";
			lang2 = "tr";
			break;

		case 0x757264: // urd
			langName = "Urdu";
			lang2 = "ur";
			break;

		case 0x766965: // vie
			langName = "Vietnamese";
			lang2 = "vi";
			break;

		case 0x756e64: // und
		default:
			langName = "Undefined";
			lang2 = "xx";
			break;
	}
}

void mp4_hls_master_playlist::hls3_master_m3u8(std::vector<std::shared_ptr<mp4_file>>& mp4files)
{
	printf("#EXTM3U\n");

	struct TrackInfo {
		std::string name;
		uint32_t track_id;
		size_t size;
		double duration;
		std::string codecs;
		uint32_t width;
		uint32_t height;
		float frame_rate;
	};

	for ( auto mp4: mp4files ) {
		size_t delim = mp4->path().find_last_of(SLASH);
		std::string file = url_encode( ( std::string::npos != delim )
				? mp4->path().substr( delim + 1 )
				: mp4->path() );

		TrackInfo video_track { "", 0, 0, 0, "", 0, 0, 0 };
		TrackInfo audio_track { "", 0, 0, 0, "", 0, 0, 0 };

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

			switch ( hdlr[0]->data().handler_type ) {
				case HandlerBox::VIDEO:
					{
						const auto& stts = select<TimeToSampleBox>(minf[0]);
						assert( 1 == stts.size() );
						assert( 1 <= stts[0]->data().entries.size() );
						if ( stts.empty() || stts[0]->data().entries.empty() ) continue;
						std::string codecs;
						if ( !decoder_conf_string(minf[0], codecs) ) {
							continue;
						}
						char buf[MAX_FILENAME];
						snprintf(buf, sizeof(buf), "%s_%u", file.c_str(), tkhd[0]->data().track_ID);
						video_track = TrackInfo{
							buf, tkhd[0]->data().track_ID, size,
								(double) mdhd[0]->data().duration / mdhd[0]->data().timescale,
								codecs,
								tkhd[0]->data().width / 0x10000,
								tkhd[0]->data().height / 0x10000,
								(float) mdhd[0]->data().timescale / stts[0]->data().entries[0].sample_delta };
						break;
					}

				case HandlerBox::AUDIO:
					{
						std::string codecs;
						if ( !decoder_conf_string(minf[0], codecs) ) {
							continue;
						}
						char buf[MAX_FILENAME];
						snprintf(buf, sizeof(buf), "%s_%u", file.c_str(), tkhd[0]->data().track_ID);
						audio_track = TrackInfo{
							buf, tkhd[0]->data().track_ID, size,
								(double) mdhd[0]->data().duration / mdhd[0]->data().timescale,
								codecs,
								0, 0,
								0 };
						break;
					}

				default:
					assert( false );
					break;
			}
		}

		if ( 0 != video_track.track_id && 0 != audio_track.track_id ) {
			printf("\n");
			printf("#EXT-X-STREAM-INF:BANDWIDTH=%lu,CODECS=\"%s,%s\",RESOLUTION=%ux%u\n",
					(uint64_t) ( (double) ( ( audio_track.size + video_track.size ) * 8 ) / video_track.duration ),
					video_track.codecs.c_str(), audio_track.codecs.c_str(),
					video_track.width, video_track.height);
			printf("%s.m3u8\n", video_track.name.c_str());
		}
		else if ( 0 != video_track.track_id ) {
			printf("\n");
			printf("#EXT-X-STREAM-INF:BANDWIDTH=%lu,CODECS=\"%s\",RESOLUTION=%ux%u\n",
					(uint64_t) ( (double) ( video_track.size * 8 ) / video_track.duration ),
					video_track.codecs.c_str(),
					video_track.width, video_track.height);
			printf("%s.m3u8\n", video_track.name.c_str());
		}
		else if ( 0 != audio_track.track_id ) {
			printf("\n");
			printf("#EXT-X-STREAM-INF:BANDWIDTH=%lu,CODECS=\"%s\"\n",
					(uint64_t) ( (double) ( audio_track.size * 8 ) / audio_track.duration ),
					audio_track.codecs.c_str());
			printf("%s.m3u8\n", audio_track.name.c_str());
		}
	}
}

void mp4_hls_master_playlist::hls7_master_m3u8(std::vector<std::shared_ptr<mp4_file>>& mp4files)
{
	printf("#EXTM3U\n");
	printf("#EXT-X-VERSION:6\n");
	printf("#EXT-X-INDEPENDENT-SEGMENTS\n");

	struct VideoTrackInfo {
		std::string name;
		uint32_t track_id;
		double duration;
		std::string codecs;
		uint32_t width;
		uint32_t height;
		float frame_rate;
	};

	struct AudioTrackInfo {
		std::string name;
		uint32_t track_id;
		double duration;
		char language[3];
		std::string codecs;
		std::string lang2;
		std::string langName;
		size_t size;
	};

	std::map<size_t, VideoTrackInfo> video_tracks;
	std::map<uint32_t, AudioTrackInfo> audio_tracks;

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
					continue;
				}
				char buf[MAX_FILENAME];
				snprintf(buf, sizeof(buf), "%s/%u", file.c_str(), tkhd[0]->data().track_ID);
				video_tracks[ size ] = VideoTrackInfo{
					buf, tkhd[0]->data().track_ID,
					(double) mdhd[0]->data().duration / mdhd[0]->data().timescale,
					codecs,
					tkhd[0]->data().width / 0x10000,
					tkhd[0]->data().height / 0x10000,
					(float) mdhd[0]->data().timescale / stts[0]->data().entries[0].sample_delta };
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

				std::string lang2;
				std::string langName;
				langCodeToName(
						mdhd[0]->data().language[0], mdhd[0]->data().language[1], mdhd[0]->data().language[2],
						lang2, langName);

				char buf[MAX_FILENAME];
				snprintf(buf, sizeof(buf), "%s/%u", file.c_str(), tkhd[0]->data().track_ID);
				audio_tracks[ tkhd[0]->data().track_ID ] = AudioTrackInfo{
					buf, tkhd[0]->data().track_ID,
					(double) mdhd[0]->data().duration / mdhd[0]->data().timescale,
					{ mdhd[0]->data().language[0], mdhd[0]->data().language[1], mdhd[0]->data().language[2], },
					codecs, lang2, langName, size };
			}
		}
	}

	// video & audio
	if ( !video_tracks.empty() && !audio_tracks.empty() ) {
		printf("\n");
		for ( auto a: audio_tracks ) {
			printf("#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID=\"%s\",NAME=\"%s %s\",AUTOSELECT=YES,DEFAULT=YES,URI=\"%s.m3u8\"\n",
					a.second.name.c_str(),
					a.second.langName.c_str(), a.second.lang2.c_str(),
					a.second.name.c_str());
		}

		unsigned int cnt = 0;
		for ( auto a: audio_tracks ) {
			++cnt;
			for ( auto v: video_tracks ) {
				printf("\n");
				printf("#EXT-X-STREAM-INF:BANDWIDTH=%lu,CODECS=\"%s,%s\",RESOLUTION=%ux%u,FRAME-RATE=%0.3f,CLOSED-CAPTIONS=\"cc%u\",AUDIO=\"%s\"\n",
						(uint64_t) ( (double) ( ( a.second.size + v.first ) * 8 ) / v.second.duration ),
						v.second.codecs.c_str(), a.second.codecs.c_str(),
						v.second.width, v.second.height,
						v.second.frame_rate,
						cnt,
						a.second.name.c_str());
				printf("%s.m3u8\n", v.second.name.c_str());
			}
		}

		cnt = 0;
		printf("\n");
		for ( auto a: audio_tracks ) {
			printf("#EXT-X-MEDIA:TYPE=CLOSED-CAPTIONS,GROUP-ID=\"cc%u\",LANGUAGE=\"%s\",NAME=\"%s\",AUTOSELECT=YES,DEFAULT=YES,INSTREAM-ID=\"CC%u\"\n",
					++cnt,
					a.second.lang2.c_str(),
					a.second.langName.c_str(),
					cnt);
		}
	}
	// video only
	else if ( !video_tracks.empty() ) {
		// vide only
		for ( auto v: video_tracks ) {
			printf("\n");
			printf("#EXT-X-STREAM-INF:BANDWIDTH=%lu,CODECS=\"%s\",RESOLUTION=%ux%u,FRAME-RATE=%0.3f\n",
					(uint64_t) ( (double) ( v.first * 8 ) / v.second.duration ),
					v.second.codecs.c_str(),
					v.second.width, v.second.height,
					v.second.frame_rate);
			printf("%s.m3u8\n", v.second.name.c_str());
		}
	}
	// audio only
	else if ( !audio_tracks.empty() ) {
		// audio only
		printf("\n");
		for ( auto a: audio_tracks ) {
			printf("#EXT-X-STREAM-INF:BANDWIDTH=%lu,CODECS=\"%s\"\n",
					(uint64_t) ( (double) ( a.second.size * 8 ) / a.second.duration ),
					a.second.codecs.c_str());
			printf("%s.m3u8\n", a.second.name.c_str());
		}
	}
}

void mp4_hls_master_playlist::execute(std::vector<std::shared_ptr<mp4_file>>& mp4files)
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

	switch ( _version ) {
		case 7:
			hls7_master_m3u8(mp4files);
			return;

		default:
			hls3_master_m3u8(mp4files);
			return;
	}

}
