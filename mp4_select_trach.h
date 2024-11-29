/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class mp4_select_track : public mp4_abstract_action {
	protected:
		class mp4_select_track_visitor : public mp4_visitor {
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
				mp4_select_track_visitor(std::shared_ptr<mp4_select_track_visitor::SelectStrategy>);
				virtual ~mp4_select_track_visitor();

			protected:
				std::shared_ptr<mp4_select_track_visitor::SelectStrategy> _selector;

			public:
				virtual void visit(BoxHead&, std::vector<std::shared_ptr<mp4_abstract_box>>&);
				virtual void visit(BoxHead&, MovieHeaderBox&);
				virtual void visit(BoxHead&, TrackHeaderBox&);
		};

	public:
		mp4_select_track(uint32_t);
		mp4_select_track(const char*);
		virtual ~mp4_select_track();

	protected:
		std::shared_ptr<mp4_select_track_visitor::SelectStrategy> _selector;

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};
