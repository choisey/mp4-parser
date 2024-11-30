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
						~select_by_number() override;

					protected:
						uint32_t _track_id;

					public:
						bool is_selected(std::shared_ptr<mp4_abstract_box>) override;
				};

				class select_by_media : public select_strategy {
					public:
						select_by_media(const char*);
						~select_by_media() override;

					protected:
						std::string _media_type;
						int _selected;

					public:
						bool is_selected(std::shared_ptr<mp4_abstract_box>) override;
				};

			public:
				mp4_select_track_visitor(std::shared_ptr<mp4_select_track_visitor::select_strategy>);
				~mp4_select_track_visitor() override;

			protected:
				std::shared_ptr<mp4_select_track_visitor::select_strategy> _selector;

			public:
				void visit(BoxHead&, std::vector<std::shared_ptr<mp4_abstract_box>>&) override;
				void visit(BoxHead&, MovieHeaderBox&) override;
				void visit(BoxHead&, TrackHeaderBox&) override;
		};

	public:
		mp4_select_track(uint32_t);
		mp4_select_track(const char*);
		~mp4_select_track() override;

	protected:
		std::shared_ptr<mp4_select_track_visitor::select_strategy> _selector;

	public:
		void execute(std::shared_ptr<mp4_abstract_box>) override;
};
