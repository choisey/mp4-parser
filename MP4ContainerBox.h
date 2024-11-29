#ifndef __MP4CONTAINERBOX_H__
#define __MP4CONTAINERBOX_H__

#include "MP4AbstractBox.h"
#include <memory>

class MP4ContainerBox : public MP4AbstractBox {
	public:
		MP4ContainerBox(const MP4ContainerBox&);
		MP4ContainerBox(const BoxHead&);
		MP4ContainerBox(uint32_t);
		virtual ~MP4ContainerBox();

	protected:
		std::vector<std::shared_ptr<MP4AbstractBox>> _boxes;

	public:
		virtual std::shared_ptr<MP4AbstractBox> clone();
		virtual void remove();
		virtual void accept(MP4Visitor*);
		virtual void select(uint32_t, std::vector<std::shared_ptr<MP4AbstractBox>>&);
		virtual void select(const std::type_info&, std::vector<std::shared_ptr<MP4AbstractBox>>&);
		virtual bool istype(const std::type_info&) const;

		MP4ContainerBox& operator=(const MP4ContainerBox&);
		virtual void addChild(std::shared_ptr<MP4AbstractBox>);
		virtual MP4AbstractBox& operator<<(std::shared_ptr<MP4AbstractBox>);
};

#endif
