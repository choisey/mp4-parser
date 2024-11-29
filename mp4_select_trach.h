/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class MP4SelectTrack : public mp4_abstract_action {
	protected:
		class MP4SelectTrackVisitor : public mp4_visitor {
			public:
				class SelectStrategy {
					public:
						SelectStrategy();
						virtual ~SelectStrategy();

					public:
						virtual bool is_selected(std::shared_ptr<mp4_abstract_box>) = 0;
				};

				class SelectByNumber : public SelectStrategy {
					public:
						SelectByNumber(uint32_t);
						virtual ~SelectByNumber();

					protected:
						uint32_t _track_id;

					public:
						virtual bool is_selected(std::shared_ptr<mp4_abstract_box>);
				};

				class SelectByMedia : public SelectStrategy {
					public:
						SelectByMedia(const char*);
						virtual ~SelectByMedia();

					protected:
						std::string _media_type;
						int _selected;

					public:
						virtual bool is_selected(std::shared_ptr<mp4_abstract_box>);
				};

			public:
				MP4SelectTrackVisitor(std::shared_ptr<MP4SelectTrackVisitor::SelectStrategy>);
				virtual ~MP4SelectTrackVisitor();

			protected:
				std::shared_ptr<MP4SelectTrackVisitor::SelectStrategy> _selector;

			public:
				virtual void visit(BoxHead&, std::vector<std::shared_ptr<mp4_abstract_box>>&);
				virtual void visit(BoxHead&, MovieHeaderBox&);
				virtual void visit(BoxHead&, TrackHeaderBox&);
		};

	public:
		MP4SelectTrack(uint32_t);
		MP4SelectTrack(const char*);
		virtual ~MP4SelectTrack();

	protected:
		std::shared_ptr<MP4SelectTrackVisitor::SelectStrategy> _selector;

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};
