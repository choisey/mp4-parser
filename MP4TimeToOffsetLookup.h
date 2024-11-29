#pragma once

#include "MP4AbstractAction.h"

class MP4TimeToOffsetLookup : public MP4AbstractAction {
	public:
		MP4TimeToOffsetLookup(uint32_t, uint64_t);
		virtual ~MP4TimeToOffsetLookup();

	protected:
		uint32_t _track_id;
		uint64_t _time;

	public:
		virtual void execute(std::shared_ptr<MP4AbstractBox>);
};
