#ifndef __MP4HLS_MEDIA_PLAYLIST_H__
#define __MP4HLS_MEDIA_PLAYLIST_H__

#include "MP4Playlist.h"

class MP4HlsMediaPlaylist : public MP4Playlist {
	public:
		MP4HlsMediaPlaylist(uint32_t, uint32_t);
		virtual ~MP4HlsMediaPlaylist();

	protected:
		uint32_t _track_id;

	public:
		virtual void execute(std::shared_ptr<MP4AbstractBox>);
		virtual void execute(std::vector<std::shared_ptr<MP4File>>&);
};

#endif
