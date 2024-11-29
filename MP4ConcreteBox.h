/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "MP4AbstractBox.h"

template <typename T>
class MP4ConcreteBox : public MP4AbstractBox {
	public:
		MP4ConcreteBox(const MP4ConcreteBox<T>& b) : MP4AbstractBox(b._head), _data(b._data) {}
		MP4ConcreteBox(const BoxHead& head) : MP4AbstractBox(head) {}
		virtual ~MP4ConcreteBox() {}

	protected:
		T _data;

	public:
		virtual std::shared_ptr<MP4AbstractBox> clone() {
			return std::shared_ptr<MP4AbstractBox>( new MP4ConcreteBox<T>(*this) );
		}

		virtual void remove() {
			_head.offset = 0;
			_head.boxsize = 0;
			_head.boxheadsize = 0;
			_head.boxtype = _XX_;
		}

		virtual void accept(MP4Visitor* visitor) {
			visitor->visit( _head, _data );
		}

		virtual void select(uint32_t, std::vector<std::shared_ptr<MP4AbstractBox>>&) {}
		virtual void select(const std::type_info& t, std::vector<std::shared_ptr<MP4AbstractBox>>&) {}
		virtual bool istype(const std::type_info& t) const { return ( typeid(T) == t ) ? true : false; }

		T& data() { return _data; }

		MP4ConcreteBox<T>& operator=(const MP4ConcreteBox<T>& b) {
			_head = b._head;
			_data = b._data;
			return *this;
		}
};
