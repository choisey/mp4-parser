/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "File.h"
#include <stdio.h>

namespace DCP {
	class LocalFile : public File
	{
		public:
			LocalFile();
			LocalFile(const std::string&);
			virtual ~LocalFile();

		protected:
			FILE* _fp;

		public:
			virtual std::shared_ptr<File::Block> allocateBlock(size_t = 0);

			virtual bool open(const std::string&);
			virtual bool is_open() const { return ( NULL != _fp ); }
			virtual off_t position() const;
			virtual bool seek(off_t, int);
			virtual size_t read(void*, size_t);
			virtual void close();
	};
};
