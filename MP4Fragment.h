/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class MP4Fragment : public MP4AbstractAction {
	protected:
		class MP4FragmentVisitor : public MP4Visitor {
			public:
				MP4FragmentVisitor(uint64_t);
				virtual ~MP4FragmentVisitor();

			protected:
				uint64_t _decode_time;

			public:
				virtual void visit(BoxHead&, std::vector<std::shared_ptr<mp4_abstract_box>>&);
		};

	public:
		MP4Fragment(uint64_t);
		virtual ~MP4Fragment();

	protected:
		uint64_t _decode_time;

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};
