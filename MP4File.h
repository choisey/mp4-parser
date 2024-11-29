#ifndef __MP4FILE_H__
#define __MP4FILE_H__

#include "MP4ContainerBox.h"

class MP4File : public MP4ContainerBox {
	public:
		MP4File(const std::string&);
		virtual ~MP4File();

	protected:
		std::string _path;

	public:
		const std::string& path() const { return _path; }
		void path(const std::string& path) { _path = path; }
};

#endif
