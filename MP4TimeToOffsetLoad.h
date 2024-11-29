#ifndef __MP4_TIME_TO_OFFSET_LOAD_H__
#define __MP4_TIME_TO_OFFSET_LOAD_H__

#include "MP4Load.h"
#include <map>

class MP4TimeToOffsetLoad : public MP4Load {
	protected:
		class MP4TimeToOffsetLoadVisitor : public MP4LoadVisitor {
			public:
				MP4TimeToOffsetLoadVisitor(std::shared_ptr<DCP::File>);
				virtual ~MP4TimeToOffsetLoadVisitor();

			public:
				virtual void visit(BoxHead&, std::vector<std::shared_ptr<MP4AbstractBox>>&);
		};

	public:
		MP4TimeToOffsetLoad(const char*);
		virtual ~MP4TimeToOffsetLoad();

	public:
		virtual void execute(std::shared_ptr<MP4AbstractBox>);
		virtual void execute(std::vector<std::shared_ptr<MP4File>>&);
};

#endif