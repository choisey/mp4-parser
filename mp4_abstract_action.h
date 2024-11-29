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
				virtual ~mp4_validate_visitor();

			protected:
				std::vector<Movie> _movies;
				std::vector<Track> _tracks;
				std::vector<std::pair<uint64_t, uint64_t>> _chunks;

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
				virtual void visit(BoxHead&, TimeToSampleBox&);
				virtual void visit(BoxHead&, CompositionOffsetBox&);
				virtual void visit(BoxHead&, SampleToChunkBox&);
				virtual void visit(BoxHead&, SampleSizeBox&);
				virtual void visit(BoxHead&, ChunkOffsetBox&);
				virtual void visit(BoxHead&, ChunkLargeOffsetBox&);
				virtual void visit(BoxHead&, SyncSampleBox&);
				virtual void visit(BoxHead&, EditListBox&);
				virtual void visit(BoxHead&, TrackFragmentHeaderBox&);
				virtual void visit(BoxHead&, TrackFragmentDecodeTimeBox&);
				virtual void visit(BoxHead&, TrackRunBox&);
				virtual void visit(BoxHead&, SegmentIndexBox&);
				virtual void visit(BoxHead&, TrackExtendsBox&);
				virtual void visit(BoxHead&, MediaDataBox&);
				virtual void visit(BoxHead&, DataBox<std::vector<uint8_t>>&);
				virtual void visit(BoxHead&, EmptyBox&);
		};
#endif

	public:
		mp4_abstract_action();
		virtual ~mp4_abstract_action();

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
