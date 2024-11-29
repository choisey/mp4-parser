/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"
#include "io_file.h"
#include <map>

class MP4Load : public MP4AbstractAction {
	protected:
		class MP4LoadVisitor : public MP4Visitor {
			public:
				MP4LoadVisitor(std::shared_ptr<io_file>);
				virtual ~MP4LoadVisitor();

			protected:
				std::shared_ptr<io_file> _f;
				std::vector<uint32_t> _handler_types;

			protected:
				template <typename T> inline std::shared_ptr<T> newBox(BoxHead& head) {
					std::shared_ptr<T> b = std::make_shared<T>(head);
					return b;
				}

				inline int16_t I16(uint8_t b0, uint8_t b1) {
					return (int16_t) ( b0 * 0x100 + b1 );
				}

				inline uint16_t U16(uint8_t b0, uint8_t b1) {
					return (uint16_t) b0 * 0x100 + (uint16_t) b1;
				}

				inline int32_t I32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
					return (int32_t) b0 * 0x1000000
						+ (int32_t) b1 * 0x10000
						+ (int32_t) b2 * 0x100
						+ (int32_t) b3;
				}

				inline uint32_t U32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
					return (uint32_t) b0 * 0x1000000
						+ (uint32_t) b1 * 0x10000
						+ (uint32_t) b2 * 0x100
						+ (uint32_t) b3;
				}

				inline int64_t I64(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7) {
					return (int64_t) b0 * 0x100000000000000L
						+ (int64_t) b1 * 0x1000000000000L
						+ (int64_t) b2 * 0x10000000000L
						+ (int64_t) b3 * 0x100000000L
						+ (int64_t) b4 * 0x1000000L
						+ (int64_t) b5 * 0x10000L
						+ (int64_t) b6 * 0x100L
						+ (int64_t) b7;
				}

				inline uint64_t U64(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7) {
					return (uint64_t) b0 * 0x100000000000000UL
						+ (uint64_t) b1 * 0x1000000000000UL
						+ (uint64_t) b2 * 0x10000000000UL
						+ (uint64_t) b3 * 0x100000000UL
						+ (uint64_t) b4 * 0x1000000UL
						+ (uint64_t) b5 * 0x10000UL
						+ (uint64_t) b6 * 0x100UL
						+ (uint64_t) b7;
				}

				bool readI32(int&);
				bool readU8(uint8_t&);
				bool readU32(uint32_t&);
				bool readU64(uint64_t&);
				bool readBoxHead(BoxHead&);
				size_t readBox(size_t, uint32_t, std::vector<std::shared_ptr<MP4AbstractBox>>&);

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
				virtual void visit(BoxHead&, TrackFragmentRandomAccessBox&);
				virtual void visit(BoxHead&, MovieFragmentRandomAccessOffsetBox&);
				virtual void visit(BoxHead&, TrackExtendsBox&);
				virtual void visit(BoxHead&, MediaDataBox&);
				virtual void visit(BoxHead&, DataBox<std::vector<uint8_t>>&);
				virtual void visit(BoxHead&, EmptyBox&);
		};

	public:
		MP4Load(const char*);
		virtual ~MP4Load();

	protected:
		std::string _uri;

	public:
		virtual void execute(std::shared_ptr<MP4AbstractBox>);
		virtual void execute(std::vector<std::shared_ptr<MP4File>>&);
};
