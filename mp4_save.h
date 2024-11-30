/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"
#include <stdio.h>

class mp4_save : public mp4_abstract_action {
	protected:
		class mp4_save_visitor : public mp4_visitor {
			public:
				mp4_save_visitor(FILE*);
				~mp4_save_visitor() override;

			private:
				FILE* _fp;

			private:
				void write8b(uint8_t);
				void write16b(uint16_t);
				void write32b(uint32_t);
				void write64b(uint64_t);
				void write4c(const char*);
				void write_string(const std::string&);
				void write_box_head(const BoxHead&);

			public:
				void visit(BoxHead&, std::vector<std::shared_ptr<mp4_abstract_box>>&) override;
				void visit(BoxHead&, FileTypeBox&) override;
				void visit(BoxHead&, MovieHeaderBox&) override;
				void visit(BoxHead&, MovieExtendsHeaderBox&) override;
				void visit(BoxHead&, TrackHeaderBox&) override;
				void visit(BoxHead&, MediaHeaderBox&) override;
				void visit(BoxHead&, HandlerBox&) override;
				void visit(BoxHead&, VideoMediaHeaderBox&) override;
				void visit(BoxHead&, SoundMediaHeaderBox&) override;
				void visit(BoxHead&, HintMediaHeaderBox&) override;
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
		mp4_save();
		mp4_save(const char*);
		~mp4_save() override;

	protected:
		std::string _filename;

	public:
		void execute(std::shared_ptr<mp4_abstract_box>) override;
};
