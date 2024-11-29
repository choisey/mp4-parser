/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_load.h"
#include <map>

class mp4_time_to_offset_load : public mp4_load {
	protected:
		class mp4_time_to_offset_load_visitor : public mp4_load_visitor {
			public:
				mp4_time_to_offset_load_visitor(std::shared_ptr<io_file>);
				virtual ~mp4_time_to_offset_load_visitor();

			public:
				virtual void visit(BoxHead&, std::vector<std::shared_ptr<mp4_abstract_box>>&);
		};

	public:
		mp4_time_to_offset_load(const char*);
		virtual ~mp4_time_to_offset_load();

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
		virtual void execute(std::vector<std::shared_ptr<mp4_file>>&);
};
