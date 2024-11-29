/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class mp4_media_stat : public mp4_abstract_action {
	public:
		mp4_media_stat();
		virtual ~mp4_media_stat();

	protected:
		struct Track {
			uint32_t id;
			uint32_t handler_type;
			uint64_t size;
			double duration;
			uint32_t bitrate;
		};

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};
