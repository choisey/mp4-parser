/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_container_box.h"

class mp4_file : public mp4_container_box {
	public:
		mp4_file(const std::string&);
		virtual ~mp4_file();

	protected:
		std::string _path;

	public:
		const std::string& path() const { return _path; }
		void path(const std::string& path) { _path = path; }
};
