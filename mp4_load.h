/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"
#include "io_file.h"
#include <map>

class mp4_load : public mp4_abstract_action {
	protected:
		class mp4_load_visitor : public mp4_visitor {
			public:
				mp4_load_visitor(std::shared_ptr<io_file>);
				~mp4_load_visitor() override;

			protected:
				std::shared_ptr<io_file> _f;
				std::vector<uint32_t> _handler_types;

			protected:
				template <typename T> inline std::shared_ptr<T> new_box(BoxHead& head) {
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

				bool read_i32(int&);
				bool read_u8(uint8_t&);
				bool read_u32(uint32_t&);
				bool read_u64(uint64_t&);
				bool read_box_head(BoxHead&);
				size_t read_box(size_t, uint32_t, std::vector<std::shared_ptr<mp4_abstract_box>>&);

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
				void visit(BoxHead&, TrackFragmentRandomAccessBox&) override;
				void visit(BoxHead&, MovieFragmentRandomAccessOffsetBox&) override;
				void visit(BoxHead&, TrackExtendsBox&) override;
				void visit(BoxHead&, MediaDataBox&) override;
				void visit(BoxHead&, DataBox<std::vector<uint8_t>>&) override;
				void visit(BoxHead&, EmptyBox&) override;
		};

	public:
		mp4_load(const char*);
		~mp4_load() override;

	protected:
		std::string _uri;

	public:
		void execute(std::shared_ptr<mp4_abstract_box>) override;
		void execute(std::vector<std::shared_ptr<mp4_file>>&) override;
};
