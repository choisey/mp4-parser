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
				virtual ~mp4_frag_key_frame_visitor();

			protected:
				size_t _mdat_size;

			public:
				virtual void visit(BoxHead&, TrackRunBox&);
				virtual void visit(BoxHead&, MediaDataBox&);
		};

	public:
		mp4_frag_key_frame();
		virtual ~mp4_frag_key_frame();

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};
