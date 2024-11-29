#ifndef __MP4SEGMENT_H__
#define __MP4SEGMENT_H__

#include "MP4AbstractAction.h"

class MP4Segment : public MP4AbstractAction {
	protected:
		struct Chunk {
			uint32_t chunk_number;
			uint64_t offset;
			uint32_t first_sample;
			uint32_t sample_count;
			uint32_t sample_description_index;
		};

	public:
		MP4Segment();
		virtual ~MP4Segment();

	private:
		bool mdia_segment_time_and_sample_range(
				std::shared_ptr<MP4AbstractBox>, uint32_t,
				std::pair<uint64_t, uint64_t>,
				std::pair<uint64_t, uint64_t>&,
				std::pair<uint32_t, uint32_t>&);
		bool sample_to_chunk(
				std::shared_ptr<MP4AbstractBox>,
				std::pair<uint32_t, uint32_t>,
				std::vector<Chunk>&);
		bool minf_divide_samples_and_chunks(
				std::shared_ptr<MP4AbstractBox>,
				std::pair<uint32_t, uint32_t>,
				const std::vector<Chunk>&, uint64_t&);

	protected:
		bool divide(
				std::shared_ptr<MP4AbstractBox>,
				uint32_t,
				std::pair<uint64_t, uint64_t>,
				uint64_t&);

		// create/update edts box in trak
		//	using timescale and duration
		void update(
				std::shared_ptr<MP4AbstractBox>,
				uint32_t, uint32_t,
				uint64_t,
				uint64_t);

		// update mdat
		//	using chunk offsets
		void update(std::shared_ptr<MP4AbstractBox> box);

	public:
		virtual void execute(std::shared_ptr<MP4AbstractBox>) = 0;
};

#endif
