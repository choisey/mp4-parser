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
		virtual ~mp4_container_box();

	protected:
		std::vector<std::shared_ptr<mp4_abstract_box>> _boxes;

	public:
		virtual std::shared_ptr<mp4_abstract_box> clone();
		virtual void remove();
		virtual void accept(mp4_visitor*);
		virtual void select(uint32_t, std::vector<std::shared_ptr<mp4_abstract_box>>&);
		virtual void select(const std::type_info&, std::vector<std::shared_ptr<mp4_abstract_box>>&);
		virtual bool istype(const std::type_info&) const;

		mp4_container_box& operator=(const mp4_container_box&);
		virtual void add_child(std::shared_ptr<mp4_abstract_box>);
		virtual mp4_abstract_box& operator<<(std::shared_ptr<mp4_abstract_box>);
};
