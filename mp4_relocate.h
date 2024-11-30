/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"
#include <stdio.h>

class mp4_relocate : public mp4_abstract_action {
	protected:
		class mp4_relocate_visitor : public mp4_visitor {
			public:
				mp4_relocate_visitor(size_t);
				~mp4_relocate_visitor() override;

			protected:
				size_t _offset;

			protected:
				void relocate_head(BoxHead&);

			public:
				void visit(BoxHead&, std::vector<std::shared_ptr<mp4_abstract_box>>&) override;
				void visit(BoxHead&, FileTypeBox&) override;
				void visit(BoxHead&, MovieHeaderBox&) override;
				void visit(BoxHead&, MovieExtendsHeaderBox&) override;
				void visit(BoxHead&, TrackHeaderBox&) override;
				void visit(BoxHead&, MediaHeaderBox&) override;
				void visit(BoxHead&, VideoMediaHeaderBox&) override;
				void visit(BoxHead&, SoundMediaHeaderBox&) override;
				void visit(BoxHead&, HintMediaHeaderBox&) override;
				void visit(BoxHead&, HandlerBox&) override;
//				void visit(BoxHead&, SampleDescriptionBox::VisualSampleEntry&) override;
//				void visit(BoxHead&, SampleDescriptionBox::AudioSampleEntry&) override;
//				void visit(BoxHead&, SampleDescriptionBox::HintSampleEntry&) override;
				void visit(BoxHead&, TimeToSampleBox&) override;
				void visit(BoxHead&, CompositionOffsetBox&) override;
				void visit(BoxHead&, SampleToChunkBox&) override;
				void visit(BoxHead&, SampleSizeBox&) override;
				void visit(BoxHead&, ChunkOffsetBox&) override;
				void visit(BoxHead&, ChunkLargeOffsetBox&) override;
				void visit(BoxHead&, SyncSampleBox&) override;
				void visit(BoxHead&, SampleDependencyTypeBox&) override;
				void visit(BoxHead&, EditListBox&) override;
				void visit(BoxHead&, TrackFragmentHeaderBox&) override;
				void visit(BoxHead&, TrackFragmentDecodeTimeBox&) override;
				void visit(BoxHead&, TrackRunBox&) override;
				void visit(BoxHead&, SegmentIndexBox&) override;
				void visit(BoxHead&, MovieFragmentHeaderBox&) override;
				void visit(BoxHead&, TrackExtendsBox&) override;
				void visit(BoxHead&, MediaDataBox&) override;
				void visit(BoxHead&, DataBox<std::vector<uint8_t>>&) override;
				void visit(BoxHead&, EmptyBox&) override;
		};

	public:
		mp4_relocate(size_t = 0);
		~mp4_relocate() override;

	protected:
		size_t _offset;

	public:
		void execute(std::shared_ptr<mp4_abstract_box>) override;
};

void Relocate(std::shared_ptr<mp4_abstract_box>, size_t = 0);
