/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class mp4_set_box : public mp4_abstract_action {
	protected:
		class mp4_set_box_visitor : public mp4_visitor {
			public:
				mp4_set_box_visitor(const std::vector<std::string>& params);
				~mp4_set_box_visitor() override;

			protected:
				std::map<std::string, std::vector<std::pair<std::string, std::string>>> _substitutions;

			public:
				void visit(BoxHead&, MediaDataBox&) override;
		};

	public:
		mp4_set_box(const std::vector<std::string>&);
		~mp4_set_box() override;

	protected:
		std::vector<std::string> _params;

	public:
		void execute(std::shared_ptr<mp4_abstract_box>) override;
};
