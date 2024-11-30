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
				~mp4_initialization_segment_visitor() override;

			protected:
				uint32_t _track_id;

			public:
				void visit(BoxHead&, std::vector<std::shared_ptr<mp4_abstract_box>>&) override;
				void visit(BoxHead&, FileTypeBox&) override;
				void visit(BoxHead&, MovieHeaderBox&) override;
				void visit(BoxHead&, TrackHeaderBox&) override;
				void visit(BoxHead&, MediaHeaderBox&) override;
				void visit(BoxHead&, TimeToSampleBox&) override;
				void visit(BoxHead&, CompositionOffsetBox&) override;
				void visit(BoxHead&, SampleToChunkBox&) override;
				void visit(BoxHead&, SampleSizeBox&) override;
				void visit(BoxHead&, ChunkOffsetBox&) override;
				void visit(BoxHead&, ChunkLargeOffsetBox&) override;
				void visit(BoxHead&, SyncSampleBox&) override;
				void visit(BoxHead&, MediaDataBox&) override;
		};

	public:
		mp4_initialization_segment(uint32_t);
		~mp4_initialization_segment() override;

	protected:
		uint32_t _track_id;

	public:
		void execute(std::vector<std::shared_ptr<mp4_file>>&) override;
};
