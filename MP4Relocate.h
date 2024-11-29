#ifndef __MP4RELOCATE_H__
#define __MP4RELOCATE_H__

#include "MP4AbstractAction.h"
#include <stdio.h>

class MP4Relocate : public MP4AbstractAction {
	protected:
		class MP4RelocateVisitor : public MP4Visitor {
			public:
				MP4RelocateVisitor(size_t);
				virtual ~MP4RelocateVisitor();

			protected:
				size_t _offset;

			protected:
				void relocateHead(BoxHead&);

			public:
				virtual void visit(BoxHead&, std::vector<std::shared_ptr<MP4AbstractBox>>&);
				virtual void visit(BoxHead&, FileTypeBox&);
				virtual void visit(BoxHead&, MovieHeaderBox&);
				virtual void visit(BoxHead&, MovieExtendsHeaderBox&);
				virtual void visit(BoxHead&, TrackHeaderBox&);
				virtual void visit(BoxHead&, MediaHeaderBox&);
				virtual void visit(BoxHead&, VideoMediaHeaderBox&);
				virtual void visit(BoxHead&, SoundMediaHeaderBox&);
				virtual void visit(BoxHead&, HintMediaHeaderBox&);
				virtual void visit(BoxHead&, HandlerBox&);
//				virtual void visit(BoxHead&, SampleDescriptionBox::VisualSampleEntry&);
//				virtual void visit(BoxHead&, SampleDescriptionBox::AudioSampleEntry&);
//				virtual void visit(BoxHead&, SampleDescriptionBox::HintSampleEntry&);
				virtual void visit(BoxHead&, TimeToSampleBox&);
				virtual void visit(BoxHead&, CompositionOffsetBox&);
				virtual void visit(BoxHead&, SampleToChunkBox&);
				virtual void visit(BoxHead&, SampleSizeBox&);
				virtual void visit(BoxHead&, ChunkOffsetBox&);
				virtual void visit(BoxHead&, ChunkLargeOffsetBox&);
				virtual void visit(BoxHead&, SyncSampleBox&);
				virtual void visit(BoxHead&, SampleDependencyTypeBox&);
				virtual void visit(BoxHead&, EditListBox&);
				virtual void visit(BoxHead&, TrackFragmentHeaderBox&);
				virtual void visit(BoxHead&, TrackFragmentDecodeTimeBox&);
				virtual void visit(BoxHead&, TrackRunBox&);
				virtual void visit(BoxHead&, SegmentIndexBox&);
				virtual void visit(BoxHead&, MovieFragmentHeaderBox&);
				virtual void visit(BoxHead&, TrackExtendsBox&);
				virtual void visit(BoxHead&, MediaDataBox&);
				virtual void visit(BoxHead&, DataBox<std::vector<uint8_t>>&);
				virtual void visit(BoxHead&, EmptyBox&);
		};

	public:
		MP4Relocate(size_t = 0);
		virtual ~MP4Relocate();

	protected:
		size_t _offset;

	public:
		virtual void execute(std::shared_ptr<MP4AbstractBox>);
};

void Relocate(std::shared_ptr<MP4AbstractBox>, size_t = 0);

#endif
