#pragma once

#include "MP4AbstractAction.h"

class MP4InitializationSegment : public MP4AbstractAction {
	protected:
		class MP4InitializationSegmentVisitor : public MP4Visitor {
			public:
				MP4InitializationSegmentVisitor(uint32_t);
				virtual ~MP4InitializationSegmentVisitor();

			protected:
				uint32_t _track_id;

			public:
				virtual void visit(BoxHead&, std::vector<std::shared_ptr<MP4AbstractBox>>&);
				virtual void visit(BoxHead&, FileTypeBox&);
				virtual void visit(BoxHead&, MovieHeaderBox&);
				virtual void visit(BoxHead&, TrackHeaderBox&);
				virtual void visit(BoxHead&, MediaHeaderBox&);
				virtual void visit(BoxHead&, TimeToSampleBox&);
				virtual void visit(BoxHead&, CompositionOffsetBox&);
				virtual void visit(BoxHead&, SampleToChunkBox&);
				virtual void visit(BoxHead&, SampleSizeBox&);
				virtual void visit(BoxHead&, ChunkOffsetBox&);
				virtual void visit(BoxHead&, ChunkLargeOffsetBox&);
				virtual void visit(BoxHead&, SyncSampleBox&);
				virtual void visit(BoxHead&, MediaDataBox&);
		};

	public:
		MP4InitializationSegment(uint32_t);
		virtual ~MP4InitializationSegment();

	protected:
		uint32_t _track_id;

	public:
		virtual void execute(std::vector<std::shared_ptr<MP4File>>&);
};
