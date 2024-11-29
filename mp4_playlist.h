/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class mp4_playlist : public mp4_abstract_action {
	public:
		mp4_playlist(uint32_t);
		virtual ~mp4_playlist();

	private:
		uint32_t _segment_duration_in_second;

	private:
		bool minf_with_stss_to_segments(
				std::shared_ptr<mp4_abstract_box>,
				uint64_t, uint32_t,
				const std::vector<uint32_t>&,
				std::vector<std::pair<uint64_t, uint64_t>>&);
		bool minf_without_stss_to_segments(
				std::shared_ptr<mp4_abstract_box>,
				uint64_t, uint32_t,
				std::vector<std::pair<uint64_t, uint64_t>>&);

	protected:
		bool minf_to_segments(
				std::shared_ptr<mp4_abstract_box>,
				uint32_t,
				std::vector<std::pair<uint64_t, uint64_t>>&);

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};
