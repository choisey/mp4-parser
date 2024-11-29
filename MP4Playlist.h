#ifndef __MP4MANIFEST_H__
#define __MP4MANIFEST_H__

#include "MP4AbstractAction.h"

class MP4Playlist : public MP4AbstractAction {
	public:
		MP4Playlist(uint32_t);
		virtual ~MP4Playlist();

	private:
		uint32_t _segment_duration_in_second;

	private:
		bool minf_with_stss_to_segments(
				std::shared_ptr<MP4AbstractBox>,
				uint64_t, uint32_t,
				const std::vector<uint32_t>&,
				std::vector<std::pair<uint64_t, uint64_t>>&);
		bool minf_without_stss_to_segments(
				std::shared_ptr<MP4AbstractBox>,
				uint64_t, uint32_t,
				std::vector<std::pair<uint64_t, uint64_t>>&);

	protected:
		bool minf_to_segments(
				std::shared_ptr<MP4AbstractBox>,
				uint32_t,
				std::vector<std::pair<uint64_t, uint64_t>>&);

	public:
		virtual void execute(std::shared_ptr<MP4AbstractBox>);
};

#endif
