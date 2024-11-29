/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_abstract_box.h"

// mp4_visitor

mp4_visitor::mp4_visitor()
{
}

mp4_visitor::~mp4_visitor()
{
}

void mp4_visitor::visit(BoxHead&, std::vector<std::shared_ptr<mp4_abstract_box>>& boxes)
{
	for ( auto child: boxes ) {
		child->accept(this);
	}
}

// mp4_abstract_box

mp4_abstract_box::mp4_abstract_box(const mp4_abstract_box& box)
	: _head(box._head)
{
}

mp4_abstract_box::mp4_abstract_box(const BoxHead& head)
	: _head(head)
{
}

mp4_abstract_box::~mp4_abstract_box()
{
}

void mp4_abstract_box::addChild(std::shared_ptr<mp4_abstract_box> box)
{
	assert( false );
}

mp4_abstract_box& mp4_abstract_box::operator<<(std::shared_ptr<mp4_abstract_box> box)
{
	assert( false );
	return *this;
}
