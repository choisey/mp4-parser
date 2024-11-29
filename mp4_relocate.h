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
				virtual ~mp4_relocate_visitor();

			protected:
				size_t _offset;

			protected:
				void relocateHead(BoxHead&);

			public:
				virtual void visit(BoxHead&, std::vector<std::shared_ptr<mp4_abstract_box>>&);
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
		mp4_relocate(size_t = 0);
		virtual ~mp4_relocate();

	protected:
		size_t _offset;

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};

void Relocate(std::shared_ptr<mp4_abstract_box>, size_t = 0);
