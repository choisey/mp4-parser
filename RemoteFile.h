#ifndef __REMOTEFILE_H__
#define __REMOTEFILE_H__

#include "File.h"
#include <curl/curl.h>
#include <map>

namespace DCP {
	class RemoteFile : public File
	{
		public:
			static size_t curl_header_callback(void*, size_t, size_t, void*);
			static size_t curl_download_callback(void*, size_t, size_t, void*);
#ifdef _DEBUG
			static int curl_debug_callback(CURL*, curl_infotype, char*, size_t, void*);
#endif

		public:
			RemoteFile();
			RemoteFile(const std::string&);
			virtual ~RemoteFile();

		protected:
			const long TIMEOUT = 30;
			const long CONNECTION_TIMEOUT = 5;
			const char* USERAGENT = "ec/fms-http-3.2";

		protected:
			CURL* _cp;

			// open()
			int _code;

			// seek() and read()
			off_t _position;

			// response body
			struct {
				void* p;
				size_t size;
				size_t filled;
			} _buffer;

		protected:
			bool onHeader(void*, size_t);
			bool onContent(void*, size_t);

		public:
			virtual std::shared_ptr<File::Block> allocateBlock(size_t = 0);

			virtual bool open(const std::string&);
			virtual bool is_open() const { return ( NULL != _cp ); }
			virtual off_t position() const { return _position; }
			virtual bool seek(off_t, int);
			virtual size_t read(void*, size_t);
			virtual void close();
	};
};

#endif
