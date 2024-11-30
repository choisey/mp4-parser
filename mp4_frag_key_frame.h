/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class mp4_frag_key_frame : public mp4_abstract_action {
	protected:
		class mp4_frag_key_frame_visitor : public mp4_visitor {
			public:
				mp4_frag_key_frame_visitor();
				~mp4_frag_key_frame_visitor() override;

			protected:
				size_t _mdat_size;

			public:
				void visit(BoxHead&, TrackRunBox&) override;
				void visit(BoxHead&, MediaDataBox&) override;
		};

	public:
		mp4_frag_key_frame();
		~mp4_frag_key_frame() override;

	public:
		void execute(std::shared_ptr<mp4_abstract_box>) override;
};
