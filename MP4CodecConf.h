#ifndef __MP4CODECINFO_H__
#define __MP4CODECINFO_H__

#include "MP4AbstractAction.h"

class MP4CodecConf {
	public:
		MP4CodecConf();
		virtual ~MP4CodecConf();

	protected:
		bool decoder_conf_string(std::shared_ptr<MP4AbstractBox>, std::string&);
};

#endif
