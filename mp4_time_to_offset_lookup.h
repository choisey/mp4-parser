/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class MP4TimeToOffsetLookup : public mp4_abstract_action {
	public:
		MP4TimeToOffsetLookup(uint32_t, uint64_t);
		virtual ~MP4TimeToOffsetLookup();

	protected:
		uint32_t _track_id;
		uint64_t _time;

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};