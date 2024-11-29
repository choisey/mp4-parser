/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_load.h"
#include <map>

class MP4TimeToOffsetLoad : public MP4Load {
	protected:
		class MP4TimeToOffsetLoadVisitor : public MP4LoadVisitor {
			public:
				MP4TimeToOffsetLoadVisitor(std::shared_ptr<io_file>);
				virtual ~MP4TimeToOffsetLoadVisitor();

			public:
				virtual void visit(BoxHead&, std::vector<std::shared_ptr<mp4_abstract_box>>&);
		};

	public:
		MP4TimeToOffsetLoad(const char*);
		virtual ~MP4TimeToOffsetLoad();

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
		virtual void execute(std::vector<std::shared_ptr<MP4File>>&);
};
