/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_box.h"
#include <memory>

class mp4_container_box : public mp4_abstract_box {
	public:
		mp4_container_box(const mp4_container_box&);
		mp4_container_box(const BoxHead&);
		mp4_container_box(uint32_t);
		~mp4_container_box() override;

	protected:
		std::vector<std::shared_ptr<mp4_abstract_box>> _boxes;

	public:
		std::shared_ptr<mp4_abstract_box> clone() override;
		void remove() override;
		void accept(mp4_visitor*) override;
		void select(uint32_t, std::vector<std::shared_ptr<mp4_abstract_box>>&) override;
		void select(const std::type_info&, std::vector<std::shared_ptr<mp4_abstract_box>>&) override;
		bool istype(const std::type_info&) const override;

		mp4_container_box& operator=(const mp4_container_box&);
		void add_child(std::shared_ptr<mp4_abstract_box>) override;
		mp4_abstract_box& operator<<(std::shared_ptr<mp4_abstract_box>) override;
};
