#include "MP4FragKeyFrame.h"
//#include "MP4.h"
#include <assert.h>

// MP4FragKeyFrameVisitor

MP4FragKeyFrame::MP4FragKeyFrameVisitor::MP4FragKeyFrameVisitor()
	: _mdat_size(0)
{
}

MP4FragKeyFrame::MP4FragKeyFrameVisitor::~MP4FragKeyFrameVisitor()
{
}

void MP4FragKeyFrame::MP4FragKeyFrameVisitor::visit(BoxHead& head, TrackRunBox& trun)
{
	if ( 1 < trun.samples.size() ) {
		trun.samples.resize(1);
		_mdat_size = trun.samples.front().sample_size;
	}
}

void MP4FragKeyFrame::MP4FragKeyFrameVisitor::visit(BoxHead& head, MediaDataBox& mdat)
{
	if ( 0 != _mdat_size ) {
		mdat.byte_ranges.clear();
		mdat.chunks.clear();

		mdat.byte_ranges[ head.offset + head.boxheadsize ] = _mdat_size;
	}
}

// MP4FragKeyFrame

MP4FragKeyFrame::MP4FragKeyFrame()
{
}

MP4FragKeyFrame::~MP4FragKeyFrame()
{
}

void MP4FragKeyFrame::execute(std::shared_ptr<MP4AbstractBox> mp4)
{
	assert( MP4FILE == mp4->head().boxtype );

	MP4FragKeyFrameVisitor v;
	mp4->accept(&v);
}
