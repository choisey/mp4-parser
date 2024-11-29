/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class mp4_initialization_segment : public mp4_abstract_action {
	protected:
		class mp4_initialization_segment_visitor : public mp4_visitor {
			public:
				mp4_initialization_segment_visitor(uint32_t);
				virtual ~mp4_initialization_segment_visitor();

			protected:
				uint32_t _track_id;

			public:
				virtual void visit(BoxHead&, std::vector<std::shared_ptr<mp4_abstract_box>>&);
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
		mp4_initialization_segment(uint32_t);
		virtual ~mp4_initialization_segment();

	protected:
		uint32_t _track_id;

	public:
		virtual void execute(std::vector<std::shared_ptr<mp4_file>>&);
};
