/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class mp4_time_to_offset_lookup : public mp4_abstract_action {
	public:
		mp4_time_to_offset_lookup(uint32_t, uint64_t);
		~mp4_time_to_offset_lookup() override;

	protected:
		uint32_t _track_id;
		uint64_t _time;

	public:
		void execute(std::shared_ptr<mp4_abstract_box>) override;
};
