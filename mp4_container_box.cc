/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_container_box.h"
#include <assert.h>

mp4_container_box::mp4_container_box(const mp4_container_box& b)
	: mp4_abstract_box(b._head)
{
	for ( auto child: b._boxes ) {
		_boxes.push_back( child->clone() );
	}
}

mp4_container_box::mp4_container_box(const BoxHead& head)
	: mp4_abstract_box(head)
{
}

mp4_container_box::mp4_container_box(uint32_t boxtype)
	: mp4_abstract_box( { 0, 0, 0, boxtype, 0, 0 } )
{
}

mp4_container_box::~mp4_container_box()
{
}

std::shared_ptr<mp4_abstract_box> mp4_container_box::clone()
{
	std::shared_ptr<mp4_abstract_box> box = std::shared_ptr<mp4_abstract_box>( new mp4_container_box( *this ) );
	return box;
}

void mp4_container_box::remove()
{
	_head.offset = 0;
	_head.boxsize = 0;
	_head.boxheadsize = 0;
	_head.boxtype = _XX_;
	_boxes.clear();
}

void mp4_container_box::accept(mp4_visitor* visitor)
{
	visitor->visit( _head,  _boxes );
}

void mp4_container_box::select(uint32_t t, std::vector<std::shared_ptr<mp4_abstract_box>>& result)
{
	for ( auto child: _boxes ) {
		if ( child->head().boxtype == t ) {
			result.push_back( child );
		}

		child->select(t, result);
	}
}

void mp4_container_box::select(const std::type_info& t, std::vector<std::shared_ptr<mp4_abstract_box>>& result)
{
	for ( auto child: _boxes ) {
		if ( child->istype(t) ) {
			result.push_back( child );
		}

		child->select(t, result);
	}
}

bool mp4_container_box::istype(const std::type_info&) const
{
	return false;
}

mp4_container_box& mp4_container_box::operator=(const mp4_container_box& b)
{
	_head = b._head;
	_boxes.clear();
	for ( auto child: b._boxes ) {
		_boxes.push_back( child->clone() );
	}
	return *this;
}

void mp4_container_box::add_child(std::shared_ptr<mp4_abstract_box> box)
{
	_boxes.push_back(box);
}

mp4_abstract_box& mp4_container_box::operator<<(std::shared_ptr<mp4_abstract_box> box)
{
	_boxes.push_back(box);
	return *this;
}
