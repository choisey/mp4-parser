/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class mp4_fragment : public mp4_abstract_action {
	protected:
		class mp4_fragment_visitor : public mp4_visitor {
			public:
				mp4_fragment_visitor(uint64_t);
				virtual ~mp4_fragment_visitor();

			protected:
				uint64_t _decode_time;

			public:
				virtual void visit(BoxHead&, std::vector<std::shared_ptr<mp4_abstract_box>>&);
		};

	public:
		mp4_fragment(uint64_t);
		virtual ~mp4_fragment();

	protected:
		uint64_t _decode_time;

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};
