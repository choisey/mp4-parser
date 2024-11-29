#pragma once

#include "MP4Playlist.h"
#include "MP4CodecConf.h"
#include "MP4URLEncode.h"

class MP4DashMpd : public MP4Playlist, public MP4CodecConf, public MP4URLEncode {
	public:
		MP4DashMpd(uint32_t);
		virtual ~MP4DashMpd();

	public:
		//virtual void execute(std::shared_ptr<MP4AbstractBox>);
		virtual void execute(std::vector<std::shared_ptr<MP4File>>&);
};
