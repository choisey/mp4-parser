/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_abstract_action.h"

class mp4_select_track : public mp4_abstract_action {
	protected:
		class mp4_select_track_visitor : public mp4_visitor {
			public:
				class select_strategy {
					public:
						select_strategy();
						virtual ~select_strategy();

					public:
						virtual bool is_selected(std::shared_ptr<mp4_abstract_box>) = 0;
				};

				class select_by_number : public select_strategy {
					public:
						select_by_number(uint32_t);
						virtual ~select_by_number();

					protected:
						uint32_t _track_id;

					public:
						virtual bool is_selected(std::shared_ptr<mp4_abstract_box>);
				};

				class select_by_media : public select_strategy {
					public:
						select_by_media(const char*);
						virtual ~select_by_media();

					protected:
						std::string _media_type;
						int _selected;

					public:
						virtual bool is_selected(std::shared_ptr<mp4_abstract_box>);
				};

			public:
				mp4_select_track_visitor(std::shared_ptr<mp4_select_track_visitor::select_strategy>);
				virtual ~mp4_select_track_visitor();

			protected:
				std::shared_ptr<mp4_select_track_visitor::select_strategy> _selector;

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
		std::shared_ptr<mp4_select_track_visitor::select_strategy> _selector;

	public:
		virtual void execute(std::shared_ptr<mp4_abstract_box>);
};
