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
		virtual ~mp4_concrete_box() {}

	protected:
		T _data;

	public:
		virtual std::shared_ptr<mp4_abstract_box> clone() {
			return std::shared_ptr<mp4_abstract_box>( new mp4_concrete_box<T>(*this) );
		}

		virtual void remove() {
			_head.offset = 0;
			_head.boxsize = 0;
			_head.boxheadsize = 0;
			_head.boxtype = _XX_;
		}

		virtual void accept(mp4_visitor* visitor) {
			visitor->visit( _head, _data );
		}

		virtual void select(uint32_t, std::vector<std::shared_ptr<mp4_abstract_box>>&) {}
		virtual void select(const std::type_info& t, std::vector<std::shared_ptr<mp4_abstract_box>>&) {}
		virtual bool istype(const std::type_info& t) const { return ( typeid(T) == t ) ? true : false; }

		T& data() { return _data; }

		mp4_concrete_box<T>& operator=(const mp4_concrete_box<T>& b) {
			_head = b._head;
			_data = b._data;
			return *this;
		}
};
