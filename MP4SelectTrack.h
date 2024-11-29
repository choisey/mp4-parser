#pragma once

#include "MP4AbstractAction.h"

class MP4SelectTrack : public MP4AbstractAction {
	protected:
		class MP4SelectTrackVisitor : public MP4Visitor {
			public:
				class SelectStrategy {
					public:
						SelectStrategy();
						virtual ~SelectStrategy();

					public:
						virtual bool is_selected(std::shared_ptr<MP4AbstractBox>) = 0;
				};

				class SelectByNumber : public SelectStrategy {
					public:
						SelectByNumber(uint32_t);
						virtual ~SelectByNumber();

					protected:
						uint32_t _track_id;

					public:
						virtual bool is_selected(std::shared_ptr<MP4AbstractBox>);
				};

				class SelectByMedia : public SelectStrategy {
					public:
						SelectByMedia(const char*);
						virtual ~SelectByMedia();

					protected:
						std::string _media_type;
						int _selected;

					public:
						virtual bool is_selected(std::shared_ptr<MP4AbstractBox>);
				};

			public:
				MP4SelectTrackVisitor(std::shared_ptr<MP4SelectTrackVisitor::SelectStrategy>);
				virtual ~MP4SelectTrackVisitor();

			protected:
				std::shared_ptr<MP4SelectTrackVisitor::SelectStrategy> _selector;

			public:
				virtual void visit(BoxHead&, std::vector<std::shared_ptr<MP4AbstractBox>>&);
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
		virtual void execute(std::shared_ptr<MP4AbstractBox>);
};
