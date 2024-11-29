/*
 * Copyright (c) Seungyeob Choi
 */

#include "MP4CodecConf.h"
#include "MP4.h"
#include <assert.h>

// SampleEntry is 8 bytes
// VisualSampleEntry is SampleEntry + 70 bytes
#define STSD_VISUALSAMPLEENTRY_SIZE	( 8 + 70 )

// MP4CodecConf

MP4CodecConf::MP4CodecConf()
{
}

MP4CodecConf::~MP4CodecConf()
{
}

// codec info
bool MP4CodecConf::decoder_conf_string(std::shared_ptr<MP4AbstractBox> minf, std::string& codecs)
{
	assert( MINF == minf->head().boxtype);

	static const uint32_t AVC1 = 0x61766331;
	static const uint32_t AVCC = 0x61766343;
	static const uint32_t HEV1 = 0x68657631;
	static const uint32_t HVCC = 0x68766343;
	static const uint32_t MP4A = 0x6d703461;
	static const uint32_t AC3  = 0x61632d33;

	struct ExtensionHead {
		size_t size;
		uint32_t type;
	};

	const auto& stsd = select(minf, STSD);
	assert( 1 == stsd.size() );
	if ( stsd.empty() ) {
		// minf has no stsd box
		return false;
	}

	for ( auto e: select< DataBox<std::vector<uint8_t>> >(stsd[0]) ) {
		switch ( e->head().boxtype ) {
			case AVC1:
			case HEV1:
				{
					auto& boxdata = e->data().boxdata;
					size_t boxdata_offset = STSD_VISUALSAMPLEENTRY_SIZE;

					while ( boxdata_offset + sizeof(uint32_t) * 2 < ( e->head().boxsize - e->head().boxheadsize ) ) {
						ExtensionHead ext;
						size_t ext_off = boxdata_offset;

						ext.size = (uint32_t) boxdata[ext_off++] * 0x1000000
							+ (uint32_t) boxdata[ext_off++] * 0x10000
							+ (uint32_t) boxdata[ext_off++] * 0x100
							+ (uint32_t) boxdata[ext_off++];
						ext.type = (uint32_t) boxdata[ext_off++] * 0x1000000
							+ (uint32_t) boxdata[ext_off++] * 0x10000
							+ (uint32_t) boxdata[ext_off++] * 0x100
							+ (uint32_t) boxdata[ext_off++];

#ifdef _DEBUG
						std::cerr << "debug:" << __FILE__ << ':' << __LINE__ << ':' << __func__ << ':';
						fprintf(stderr, "stsd/%c%c%c%c[%lu/%lu]/%c%c%c%c[%lu/%lu]\n", 
								(char) ( e->head().boxtype / 0x1000000 ),
								(char) ( ( e->head().boxtype / 0x10000 ) & 0xff ),
								(char) ( ( e->head().boxtype / 0x100 ) & 0xff ),
								(char) ( e->head().boxtype & 0xff ),
								e->head().offset,
								e->head().boxsize - e->head().boxheadsize,
								(char) ( ext.type / 0x1000000 ),
								(char) ( ( ext.type / 0x10000 ) & 0xff ),
								(char) ( ( ext.type / 0x100 ) & 0xff ),
								(char) ( ext.type & 0xff ),
								boxdata_offset,
								ext.size);
#endif

						// avcC
						switch ( ext.type ) {
							case AVCC:
								{
									char buf[40];
									snprintf(buf, sizeof(buf), "%c%c%c%c.%02x%02x%02x",
											(char) ( e->head().boxtype / 0x1000000 ),
											(char) ( ( e->head().boxtype / 0x10000 ) & 0xff ),
											(char) ( ( e->head().boxtype / 0x100 ) & 0xff ),
											(char) ( e->head().boxtype & 0xff ),
											boxdata[ext_off + 1], boxdata[ext_off + 2], boxdata[ext_off + 3]);
									codecs = buf;
									return true;
								}
						}

						boxdata_offset += ext.size;
					}

					return false;
				}

			case MP4A:
				codecs = "mp4a.40.2";
				return true;

			case AC3:
				codecs = "ac3";
				return true;
		}
	}

	codecs="";
	return false;
}
