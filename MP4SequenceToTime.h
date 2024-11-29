#pragma once

#include "MP4MultiTrackSegment.h"
#include "MP4Playlist.h"

class MP4SequenceToTime : public MP4Playlist {
	public:
		MP4SequenceToTime(uint32_t);
		virtual ~MP4SequenceToTime();

	public:
		virtual void execute(std::shared_ptr<MP4AbstractBox>);

		uint64_t multi_track_segment_time(std::shared_ptr<MP4AbstractBox>, uint32_t);
		uint64_t single_track_segment_time(std::shared_ptr<MP4AbstractBox>, uint32_t, uint32_t);
};
