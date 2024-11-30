/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_object.h"
#include "mp4_abstract_box.h"
#include "mp4_file.h"

#define MIN_DURATION_OF_LAST_SEGMENT	1

class mp4_abstract_action : public mp4_object {
#ifdef _DEBUG
	protected:
		class mp4_validate_visitor : public mp4_visitor {
			protected:
				struct Chunk {
					uint64_t offset;
					uint64_t size;
				};

				struct Movie {
					std::vector<MovieHeaderBox> mvhd;
				};

				struct Track {
					uint32_t track_type;
					std::vector<TrackHeaderBox> tkhd;
					std::vector<MediaHeaderBox> mdhd;

					std::vector<SampleToChunkBox::Entry> sample_to_chunks;
					std::vector<uint64_t> chunk_offsets;
					std::vector<uint32_t> sample_sizes;
					std::vector<uint32_t> sync_samples;
					std::vector<TimeToSampleBox::Entry> time_to_samples;
					std::vector<CompositionOffsetBox::Entry> composition_offsets;
				};

			public:
				mp4_validate_visitor();
				~mp4_validate_visitor() override;

			protected:
				std::vector<Movie> _movies;
				std::vector<Track> _tracks;
				std::vector<std::pair<uint64_t, uint64_t>> _chunks;

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
				void visit(BoxHead&, TimeToSampleBox&) override;
				void visit(BoxHead&, CompositionOffsetBox&) override;
				void visit(BoxHead&, SampleToChunkBox&) override;
				void visit(BoxHead&, SampleSizeBox&) override;
				void visit(BoxHead&, ChunkOffsetBox&) override;
				void visit(BoxHead&, ChunkLargeOffsetBox&) override;
				void visit(BoxHead&, SyncSampleBox&) override;
				void visit(BoxHead&, EditListBox&) override;
				void visit(BoxHead&, TrackFragmentHeaderBox&) override;
				void visit(BoxHead&, TrackFragmentDecodeTimeBox&) override;
				void visit(BoxHead&, TrackRunBox&) override;
				void visit(BoxHead&, SegmentIndexBox&) override;
				void visit(BoxHead&, TrackExtendsBox&) override;
				void visit(BoxHead&, MediaDataBox&) override;
				void visit(BoxHead&, DataBox<std::vector<uint8_t>>&) override;
				void visit(BoxHead&, EmptyBox&) override;
		};
#endif

	public:
		mp4_abstract_action();
		~mp4_abstract_action() override;

#ifdef _DEBUG
	protected:
		struct context {
			std::vector<std::pair<uint64_t, uint64_t>> mdat;
			struct {
				uint32_t timescale;
				uint64_t duration;
			} mvhd;
		};

	protected:
		bool validate_moov(std::shared_ptr<mp4_abstract_box>, context&);
		bool validate_trak(std::shared_ptr<mp4_abstract_box>, context&);
		bool validate_mdia(std::shared_ptr<mp4_abstract_box>, context&);
		bool validate_edts(std::shared_ptr<mp4_abstract_box>, std::shared_ptr<mp4_abstract_box>, context&);
		bool validate(std::shared_ptr<mp4_abstract_box>, context&);
#endif

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
		virtual void execute(std::vector<std::shared_ptr<mp4_file>>&);
};
