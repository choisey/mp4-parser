/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_container_box.h"

class MP4File : public mp4_container_box {
	public:
		MP4File(const std::string&);
		virtual ~MP4File();

	protected:
		std::string _path;

	public:
		const std::string& path() const { return _path; }
		void path(const std::string& path) { _path = path; }
};
