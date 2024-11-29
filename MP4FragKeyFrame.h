#ifndef __MP4_FRAG_KEYFRAME_H__
#define __MP4_FRAG_KEYFRAME_H__

#include "MP4AbstractAction.h"

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

#endif