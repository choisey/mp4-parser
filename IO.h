#ifndef __IO_H__
#define __IO_H__

#include "DCP.h"
#include "File.h"
#include <memory>

namespace DCP {
	class IO : public Object {
		public:
			static IO* Instance();
			static void cleanup();

		private:
			static IO* _instance;
			IO();

		public:
			~IO();

		public:
			std::shared_ptr<File> open(const std::string&);
	};
};

#endif
