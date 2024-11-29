/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "MP4AbstractAction.h"
#include <stdio.h>

class MP4Save : public MP4AbstractAction {
	protected:
		class MP4SaveVisitor : public MP4Visitor {
			public:
				MP4SaveVisitor(FILE*);
				virtual ~MP4SaveVisitor();

			private:
				FILE* _fp;

			private:
				void write8b(uint8_t);
				void write16b(uint16_t);
				void write32b(uint32_t);
				void write64b(uint64_t);
				void write4C(const char*);
				void writeString(const std::string&);
				void writeBoxHead(const BoxHead&);

			public:
				virtual void visit(BoxHead&, std::vector<std::shared_ptr<MP4AbstractBox>>&);
				virtual void visit(BoxHead&, FileTypeBox&);
				virtual void visit(BoxHead&, MovieHeaderBox&);
				virtual void visit(BoxHead&, MovieExtendsHeaderBox&);
				virtual void visit(BoxHead&, TrackHeaderBox&);
				virtual void visit(BoxHead&, MediaHeaderBox&);
				virtual void visit(BoxHead&, HandlerBox&);
				virtual void visit(BoxHead&, VideoMediaHeaderBox&);
				virtual void visit(BoxHead&, SoundMediaHeaderBox&);
				virtual void visit(BoxHead&, HintMediaHeaderBox&);
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
		MP4Save();
		MP4Save(const char*);
		virtual ~MP4Save();

	protected:
		std::string _filename;

	public:
		virtual void execute(std::shared_ptr<MP4AbstractBox>);
};
