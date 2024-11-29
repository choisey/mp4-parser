/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class MP4FragKeyFrame : public MP4AbstractAction {
	protected:
		class MP4FragKeyFrameVisitor : public MP4Visitor {
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
		virtual void execute(std::shared_ptr<MP4AbstractBox>);
};
