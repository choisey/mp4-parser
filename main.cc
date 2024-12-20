/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4.h"
#include "mp4_file.h"
#include "mp4_load.h"
#include "mp4_save.h"
#include "mp4_dump.h"
#include "mp4_hls_m3u8.h"
#include "mp4_hls_master_playlist.h"
#include "mp4_hls_media_playlist.h"
#include "mp4_multi_track_segment.h"
#include "mp4_multi_track_segment_by_sequence_number.h"
#include "mp4_dash_mpd.h"
#include "mp4_initialization_segment.h"
#include "mp4_fragment.h"
#include "mp4_single_track_segment.h"
#include "mp4_single_track_segment_by_sequence_number.h"
#include "mp4_time_to_offset_load.h"
#include "mp4_time_to_offset_lookup.h"
#include "mp4_set_box.h"
#include "mp4_frag_key_frame.h"
#include "mp4_media_stat.h"

#include <stdio.h>
#include <string.h>
#include <vector>
#include <memory>

int main(int argc, char* argv[])
{
	std::vector<std::unique_ptr<mp4_abstract_action>> actions;

	for ( int i = 1; i < argc; i++ ) {

		if ( '-' != *argv[i] ) {
			fprintf(stderr, "%s: unknown action\n", argv[i]);
			exit(1);
		}

		int cparams = 0;
		for ( int j = i + 1; j < argc; j++ ) {
			if ( '-' == *argv[j] ) {
				break;
			}
			++cparams;
		}

		if ( 0 == strcmp(argv[i], "--input") || 0 == strcmp(argv[i], "-i") ) {
			if ( 1 != cparams ) {
				fprintf(stderr, "%s <input-file>\n", argv[i]);
				exit(1);
			}

			actions.push_back(
					std::unique_ptr<mp4_abstract_action>(
						new mp4_load(argv[++i])
						)
					);
		}
		else if ( 0 == strcmp(argv[i], "--manifest") || 0 == strcmp(argv[i], "-m") ) {
			if ( 1 != cparams ) {
				fprintf(stderr, "%s <segment-duration>\n", argv[i]);
				exit(1);
			}

			uint32_t d = std::stoul(argv[++i]);
			actions.push_back(
					std::unique_ptr<mp4_abstract_action>(
						new mp4_playlist(d)
						)
					);
		}
		else if ( 0 == strcmp(argv[i], "--m3u8") ) {
			switch ( cparams) {
				case 0:
					{
						// HLS version 6
						// master playlist
						actions.push_back(
								std::unique_ptr<mp4_abstract_action>(
									new mp4_hls_master_playlist()
									)
								);
						break;
					}

				case 1:
					{
						// HLS version 3
						// media playlist (mpeg2ts containing video + audio combined)
						uint32_t d = std::stoul(argv[++i]);
						actions.push_back(
								std::unique_ptr<mp4_abstract_action>(
									new mp4_hls_m3u8(d)
									)
								);
						break;
					}

				case 2:
					{
						// HLS version 7
						// media playlist (fragmented mp4 containing a single track)
						uint32_t track_id = std::stoul(argv[++i]);
						uint32_t d = std::stoul(argv[++i]);
						actions.push_back(
								std::unique_ptr<mp4_abstract_action>(
									new mp4_hls_media_playlist(track_id, d)
									)
								);
						break;
					}
				default:
					fprintf(stderr, "usage: %s\n", argv[i]);
					fprintf(stderr, "       %s <segment-duration>\n", argv[i]);
					fprintf(stderr, "       %s <track-id> <segment-duration>\n", argv[i]);
					exit(1);
			}
		}
		else if ( 0 == strcmp(argv[i], "--master-m3u8") ) {
			if ( 1 < cparams ) {
				fprintf(stderr, "usage: %s <3|7>?\n", argv[i]);
				exit(1);
			}
			uint32_t v = ( 1 == cparams ) ? std::stoul(argv[++i]) : 7;
			actions.push_back(
					std::unique_ptr<mp4_abstract_action>(
						new mp4_hls_master_playlist( v )
						)
					);
			break;
		}
		else if ( 0 == strcmp(argv[i], "--mpd") ) {
			if ( 1 != cparams ) {
				fprintf(stderr, "%s <segment-duration>\n", argv[i]);
				exit(1);
			}

			uint32_t d = std::stoul(argv[++i]);
			actions.push_back(
					std::unique_ptr<mp4_abstract_action>(
						new mp4_dash_mpd(d)
						)
					);
		}
		else if ( 0 == strcmp(argv[i], "--dash-init") || 0 == strcmp(argv[i], "--init") || 0 == strcmp(argv[i], "-x") ) {
			if ( 1 != cparams ) {
				fprintf(stderr, "%s <track-id>\n", argv[i]);
				exit(1);
			}

			uint32_t track_id = std::stoul(argv[++i]);
			actions.push_back(
					std::unique_ptr<mp4_abstract_action>(
						new mp4_initialization_segment(track_id)
						)
					);
		}
		else if ( 0 == strcmp(argv[i], "--fragment") || 0 == strcmp(argv[i], "-f") ) {
			switch ( cparams ) {
				case 3:
					{
						uint32_t track_id = std::stoul(argv[++i]);
						uint64_t start = std::stoull(argv[++i]);
						uint32_t duration = std::stoul(argv[++i]);
						actions.push_back(
								std::unique_ptr<mp4_abstract_action>(
									new mp4_single_track_segment(track_id, start, duration)
									)
								);
						actions.push_back(
								std::unique_ptr<mp4_abstract_action>(
									new mp4_fragment(start)
									)
								);
						break;
					}

				default:
					fprintf(stderr, "%s <track-id> <start-time(timescale)> <duration(second)>\n", argv[i]);
					fprintf(stderr, "%s <start-time(timescale)>\n", argv[i]);
					exit(1);
			}
		}
		else if ( 0 == strcmp(argv[i], "-s") ) {
			switch ( cparams ) {
				case 2:
					{
						uint64_t start = std::stoull(argv[++i]);
						uint32_t duration = std::stoul(argv[++i]);
						actions.push_back(
								std::unique_ptr<mp4_abstract_action>(
									new mp4_multi_track_segment(start, duration)
									)
								);
						break;
					}

				case 3:
					{
						uint32_t track_id = std::stoul(argv[++i]);
						uint64_t start = std::stoull(argv[++i]);
						uint32_t duration = std::stoul(argv[++i]);
						actions.push_back(
								std::unique_ptr<mp4_abstract_action>(
									new mp4_single_track_segment(track_id, start, duration)
									)
								);
						break;
					}

				default:
					fprintf(stderr, "%s <start-time(millisecond)> <duration(second)>\n", argv[i]);
					fprintf(stderr, "%s <track-id> <start-time(timescale)> <duration(second)>\n", argv[i]);
					exit(1);
			}
		}
		else if ( 0 == strcmp(argv[i], "-S") ) {
			switch ( cparams ) {
				case 2:
					{
						uint32_t seq = std::stoul(argv[++i]);
						uint32_t duration = std::stoul(argv[++i]);
						actions.push_back(
								std::unique_ptr<mp4_abstract_action>(
									new mp4_multi_track_segment_by_sequence_number(seq, duration)
									)
								);
						break;
					}

				default:
					fprintf(stderr, "%s <pos(N)> <duration(second)>\n", argv[i]);
					exit(1);
			}
		}
		//
		// Microsoft Smooth Streaming
		//
		else if ( 0 == strcmp(argv[i], "--mfra-input") || 0 == strcmp(argv[i], "-r") ) {
			if ( 1 != cparams ) {
				fprintf(stderr, "%s <input-file>\n", argv[i]);
				exit(1);
			}

			actions.push_back(
					std::unique_ptr<mp4_abstract_action>(
						new mp4_time_to_offset_load(argv[++i])
						)
					);
		}
		else if ( 0 == strcmp(argv[i], "--mfra-lookup") || 0 == strcmp(argv[i], "-l") ) {
			if ( 2 != cparams ) {
				fprintf(stderr, "%s <track-id> <start-time(timescale)>\n", argv[i]);
				exit(1);
			}

			uint64_t track_id = std::stoul(argv[++i]);
			uint64_t time = std::stoull(argv[++i]);
			actions.push_back(
					std::unique_ptr<mp4_abstract_action>(
						new mp4_time_to_offset_lookup(track_id, time)
						)
					);
		}
		else if ( 0 == strcmp(argv[i], "--frag-keygrame") || 0 == strcmp(argv[i], "-kf") ) {
			if ( 0 != cparams ) {
				fprintf(stderr, "%s doesn't take any parameters.\n", argv[i]);
				exit(1);
			}

			actions.push_back(
					std::unique_ptr<mp4_abstract_action>(
						new mp4_frag_key_frame()
						)
					);
		}
		//
		// Dump (for test/debug purpose)
		//
		else if ( 0 == strcmp(argv[i], "--dump") || 0 == strcmp(argv[i], "-d") ) {
			if ( 0 != cparams ) {
				fprintf(stderr, "%s doesn't take any parameters.\n", argv[i]);
				exit(1);
			}

			actions.push_back(
					std::unique_ptr<mp4_abstract_action>(
						new mp4_dump()
						)
					);
		}
		else if ( 0 == strcmp(argv[i], "--media-stat") || 0 == strcmp(argv[i], "-st") ) {
			if ( 0 != cparams ) {
				fprintf(stderr, "%s doesn't take any parameters.\n", argv[i]);
				exit(1);
			}

			actions.push_back(
					std::unique_ptr<mp4_abstract_action>(
						new mp4_media_stat()
						)
					);
		}
		else if ( 0 == strcmp(argv[i], "--set") ) {
			if ( cparams < 1 ) {
				fprintf(stderr, "%s \"<box_type>:<field_name>=<value>\"\n", argv[i]);
				exit(1);
			}

			std::vector<std::string> params;

			for ( ; 0 < cparams; --cparams ) {
				params.push_back(argv[++i]);
			}

			actions.push_back(
					std::unique_ptr<mp4_abstract_action>(
						new mp4_set_box(params)
						)
					);
		}
		else if ( 0 == strcmp(argv[i], "--output") || 0 == strcmp(argv[i], "-o") ) {
			if ( 0 == cparams ) {
				actions.push_back(
						std::unique_ptr<mp4_abstract_action>(
							new mp4_save()
							)
						);
			}
			else {
				for ( ; 0 < cparams; --cparams ) {
					actions.push_back(
							std::unique_ptr<mp4_abstract_action>(
								new mp4_save(argv[++i])
								)
							);
				}
			}
		}
		else if ( 0 == strcmp(argv[i], "--help") || 0 == strcmp(argv[i], "-h") ) {
			if ( 0 != cparams ) {
				fprintf(stderr, "%s doesn't take any parameters.\n", argv[i]);
				exit(1);
			}

			printf("usage:\n");
			printf("\t-i <1>          \tLoad an mp4 file from file <1>.\n");
			printf("\t-m <1>          \tList of segment start times and durations. <1> is target duration of segment in second.\n");
			printf("\t--m3u8 <1>      \tHLS m3u8 manifest. <1> is target duration of segment in second.\n");
			printf("\t--mpd <1>       \tMPEG-DASH mpd manifest. <1> is target duration of segment in second.\n");
			printf("\t-s <1> <2>      \tCreate a multi-track segment starting at <1> (millisecond) with duration <2> (second).\n");
			printf("\t-S <1> <2>      \tCreate a multi-track segment at <1>-th position from the beginning with duration <2> (second).\n");
			printf("\t-s <1> <2> <3>  \tCreate a single-track segment from track <1> starting at <2> (timescale) with duration <3> (second).\n");
			printf("\t-S <1> <2> <3>  \tCreate a single-track segment from track <1> at <2>-th position from the befginning with duration <3> (second).\n");
			printf("\t-f <1> <2> <3>  \tCreate a single-track fragment from track <1> starting at <2> (timescale) with duration <3> (second).\n");
			printf("\t-o <1>          \tSave the resulting mp4 data in a file <1>.\n");
			printf("\t-d              \tDump the content of mp4.\n");
		}
		else {
			fprintf(stderr, "%s: unknown action\n", argv[i]);
			exit(1);
		}
	}

	// execute commands

	std::vector<std::shared_ptr<mp4_file>> mp4files;

	for ( auto iter = actions.begin(); iter != actions.end(); iter++ ) {
		(*iter)->execute(mp4files);
	}
}
