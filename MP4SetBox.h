/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class MP4SetBox : public MP4AbstractAction {
	protected:
		class MP4SetBoxVisitor : public MP4Visitor {
			public:
				MP4SetBoxVisitor(const std::vector<std::string>& params);
				virtual ~MP4SetBoxVisitor();

			protected:
				std::map<std::string, std::vector<std::pair<std::string, std::string>>> _substitutions;

			public:
				virtual void visit(BoxHead&, MediaDataBox&);
		};

	public:
		MP4SetBox(const std::vector<std::string>&);
		virtual ~MP4SetBox();

	protected:
		std::vector<std::string> _params;

	public:
		virtual void execute(std::shared_ptr<MP4AbstractBox>);
};
