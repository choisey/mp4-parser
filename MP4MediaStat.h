#ifndef __MP4MEDTASTAT_H__
#define __MP4MEDTASTAT_H__

#include "MP4AbstractAction.h"

class MP4MediaStat : public MP4AbstractAction {
	public:
		MP4MediaStat();
		virtual ~MP4MediaStat();

	protected:
		struct Track {
			uint32_t id;
			uint32_t handler_type;
			uint64_t size;
			double duration;
			uint32_t bitrate;
		};

	public:
		virtual void execute(std::shared_ptr<MP4AbstractBox>);
};

#endif
