/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_box.h"

template <typename T>
class mp4_concrete_box : public mp4_abstract_box {
	public:
		mp4_concrete_box(const mp4_concrete_box<T>& b) : mp4_abstract_box(b._head), _data(b._data) {}
		mp4_concrete_box(const BoxHead& head) : mp4_abstract_box(head) {}
		~mp4_concrete_box() override {}

	protected:
		T _data;

	public:
		std::shared_ptr<mp4_abstract_box> clone() override {
			return std::shared_ptr<mp4_abstract_box>( new mp4_concrete_box<T>(*this) );
		}

		void remove() override {
			_head.offset = 0;
			_head.boxsize = 0;
			_head.boxheadsize = 0;
			_head.boxtype = _XX_;
		}

		void accept(mp4_visitor* visitor) override {
			visitor->visit( _head, _data );
		}

		void select(uint32_t, std::vector<std::shared_ptr<mp4_abstract_box>>&) override {}
		void select(const std::type_info& t, std::vector<std::shared_ptr<mp4_abstract_box>>&) override {}
		bool istype(const std::type_info& t) const override { return ( typeid(T) == t ) ? true : false; }

		T& data() { return _data; }

		mp4_concrete_box<T>& operator=(const mp4_concrete_box<T>& b) {
			_head = b._head;
			_data = b._data;
			return *this;
		}
};
