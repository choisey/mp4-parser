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
				~mp4_time_to_offset_load_visitor() override;

			public:
				void visit(BoxHead&, std::vector<std::shared_ptr<mp4_abstract_box>>&) override;
		};

	public:
		mp4_time_to_offset_load(const char*);
		~mp4_time_to_offset_load() override;

	public:
		void execute(std::shared_ptr<mp4_abstract_box>) override;
		void execute(std::vector<std::shared_ptr<mp4_file>>&) override;
};
