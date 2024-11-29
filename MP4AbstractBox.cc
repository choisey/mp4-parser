#include "MP4AbstractBox.h"

// MP4Visitor

MP4Visitor::MP4Visitor()
{
}

MP4Visitor::~MP4Visitor()
{
}

void MP4Visitor::visit(BoxHead&, std::vector<std::shared_ptr<MP4AbstractBox>>& boxes)
{
	for ( auto child: boxes ) {
		child->accept(this);
	}
}

// MP4AbstractBox

MP4AbstractBox::MP4AbstractBox(const MP4AbstractBox& box)
	: _head(box._head)
{
}

MP4AbstractBox::MP4AbstractBox(const BoxHead& head)
	: _head(head)
{
}

MP4AbstractBox::~MP4AbstractBox()
{
}

void MP4AbstractBox::addChild(std::shared_ptr<MP4AbstractBox> box)
{
	assert( false );
}

MP4AbstractBox& MP4AbstractBox::operator<<(std::shared_ptr<MP4AbstractBox> box)
{
	assert( false );
	return *this;
}
