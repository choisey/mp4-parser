/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class MP4FragKeyFrame : public mp4_abstract_action {
	protected:
		class MP4FragKeyFrameVisitor : public mp4_visitor {
			public:
				MP4FragKeyFrameVisitor();
				virtual ~MP4FragKeyFrameVisitor();

			protected:
				size_t _mdat_size;

			public:
				virtual void visit(BoxHead&, TrackRunBox&);
				virtual void visit(BoxHead&, MediaDataBox&);
		};

	public:
		MP4FragKeyFrame();
		virtual ~MP4FragKeyFrame();

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};
