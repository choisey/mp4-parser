#include "MP4ContainerBox.h"
#include <assert.h>

MP4ContainerBox::MP4ContainerBox(const MP4ContainerBox& b)
	: MP4AbstractBox(b._head)
{
	for ( auto child: b._boxes ) {
		_boxes.push_back( child->clone() );
	}
}

MP4ContainerBox::MP4ContainerBox(const BoxHead& head)
	: MP4AbstractBox(head)
{
}

MP4ContainerBox::MP4ContainerBox(uint32_t boxtype)
	: MP4AbstractBox( { 0, 0, 0, boxtype, 0, 0 } )
{
}

MP4ContainerBox::~MP4ContainerBox()
{
}

std::shared_ptr<MP4AbstractBox> MP4ContainerBox::clone()
{
	std::shared_ptr<MP4AbstractBox> box = std::shared_ptr<MP4AbstractBox>( new MP4ContainerBox( *this ) );
	return box;
}

void MP4ContainerBox::remove()
{
	_head.offset = 0;
	_head.boxsize = 0;
	_head.boxheadsize = 0;
	_head.boxtype = _XX_;
	_boxes.clear();
}

void MP4ContainerBox::accept(MP4Visitor* visitor)
{
	visitor->visit( _head,  _boxes );
}

void MP4ContainerBox::select(uint32_t t, std::vector<std::shared_ptr<MP4AbstractBox>>& result)
{
	for ( auto child: _boxes ) {
		if ( child->head().boxtype == t ) {
			result.push_back( child );
		}

		child->select(t, result);
	}
}

void MP4ContainerBox::select(const std::type_info& t, std::vector<std::shared_ptr<MP4AbstractBox>>& result)
{
	for ( auto child: _boxes ) {
		if ( child->istype(t) ) {
			result.push_back( child );
		}

		child->select(t, result);
	}
}

bool MP4ContainerBox::istype(const std::type_info&) const
{
	return false;
}

MP4ContainerBox& MP4ContainerBox::operator=(const MP4ContainerBox& b)
{
	_head = b._head;
	_boxes.clear();
	for ( auto child: b._boxes ) {
		_boxes.push_back( child->clone() );
	}
	return *this;
}

void MP4ContainerBox::addChild(std::shared_ptr<MP4AbstractBox> box)
{
	_boxes.push_back(box);
}

MP4AbstractBox& MP4ContainerBox::operator<<(std::shared_ptr<MP4AbstractBox> box)
{
	_boxes.push_back(box);
	return *this;
}
