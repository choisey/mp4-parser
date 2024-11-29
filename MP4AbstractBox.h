/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_object.h"
#include "MP4BoxTypes.h"
#include <assert.h>
#include <typeinfo>
#include <memory>

class MP4AbstractBox;

class MP4Visitor : public MP4Object {
	public:
		MP4Visitor();
		virtual ~MP4Visitor();

	public:
		virtual void visit(BoxHead&, std::vector<std::shared_ptr<MP4AbstractBox>>&);
		virtual void visit(BoxHead&, FileTypeBox&) {}
		virtual void visit(BoxHead&, MovieHeaderBox&) {}
		virtual void visit(BoxHead&, MovieExtendsHeaderBox&) {}
		virtual void visit(BoxHead&, TrackHeaderBox&) {}
		virtual void visit(BoxHead&, MediaHeaderBox&) {}
		virtual void visit(BoxHead&, HandlerBox&) {}
		virtual void visit(BoxHead&, VideoMediaHeaderBox&) {}
		virtual void visit(BoxHead&, SoundMediaHeaderBox&) {}
		virtual void visit(BoxHead&, HintMediaHeaderBox&) {}
//		virtual void visit(BoxHead&, SampleDescriptionBox::VisualSampleEntry&) {}
//		virtual void visit(BoxHead&, SampleDescriptionBox::AudioSampleEntry&) {}
//		virtual void visit(BoxHead&, SampleDescriptionBox::HintSampleEntry&) {}
		virtual void visit(BoxHead&, TimeToSampleBox&) {}
		virtual void visit(BoxHead&, CompositionOffsetBox&) {}
		virtual void visit(BoxHead&, SampleToChunkBox&) {}
		virtual void visit(BoxHead&, SampleSizeBox&) {}
		virtual void visit(BoxHead&, ChunkOffsetBox&) {}
		virtual void visit(BoxHead&, ChunkLargeOffsetBox&) {}
		virtual void visit(BoxHead&, SyncSampleBox&) {}
		virtual void visit(BoxHead&, SampleDependencyTypeBox&) {}
		virtual void visit(BoxHead&, EditListBox&) {}
		virtual void visit(BoxHead&, TrackFragmentHeaderBox&) {}
		virtual void visit(BoxHead&, TrackFragmentDecodeTimeBox&) {}
		virtual void visit(BoxHead&, TrackRunBox&) {}
		virtual void visit(BoxHead&, SegmentIndexBox&) {}
		virtual void visit(BoxHead&, MovieFragmentHeaderBox&) {}
		virtual void visit(BoxHead&, TrackFragmentRandomAccessBox&) {}
		virtual void visit(BoxHead&, MovieFragmentRandomAccessOffsetBox&) {}
		virtual void visit(BoxHead&, TrackExtendsBox&) {}
		virtual void visit(BoxHead&, MediaDataBox&) {}
		virtual void visit(BoxHead&, DataBox<std::vector<uint8_t>>&) {}
		virtual void visit(BoxHead&, EmptyBox&) {}
};

class MP4AbstractBox : public MP4Object {
	public:
		MP4AbstractBox(const MP4AbstractBox&);
		MP4AbstractBox(const BoxHead&);
		virtual ~MP4AbstractBox();

	protected:
		BoxHead _head;

	public:
		BoxHead& head() { return _head; }

		virtual std::shared_ptr<MP4AbstractBox> clone() = 0;
		virtual void remove() = 0;
		virtual void accept(MP4Visitor*) = 0;
		virtual void select(uint32_t, std::vector<std::shared_ptr<MP4AbstractBox>>&) = 0;
		virtual void select(const std::type_info&, std::vector<std::shared_ptr<MP4AbstractBox>>&) = 0;
		virtual bool istype(const std::type_info&) const = 0;

		virtual void addChild(std::shared_ptr<MP4AbstractBox>);
		virtual MP4AbstractBox& operator<<(std::shared_ptr<MP4AbstractBox>);
};
