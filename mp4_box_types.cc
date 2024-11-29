/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_box_types.h"
#include <assert.h>

bool BoxHead::is_fullbox() const
{
	switch ( boxtype )
	{
		case ABST:
		case CO64:
		case CPRT:
		case CTTS:
		case DREF:
		case ELST:
		case HDLR:
		case HMHD:
		case ILOC:
		case IMIF:
		case INFE:
		case IPMC:
		case MDHD:
		case MEHD:
		case META:
		case MFHD:
		case MFRO:
		case MVHD:
		case NMHD:
		case PDIN:
		case PITM:
		case SBGP:
		case SDTP:
		case SGPD:
		case SIDX:
		case SMHD:
		case SRPP:
		case STCO:
		case STDP:
		case STSC:
		case STSD:
		case STSH:
		case STSL:
		case STSS:
		case STSZ:
		case STTS:
		case STZ2:
		case SUBS:
		case TFHD:
		case TFRA:
		case TKHD:
		case TREX:
		case TRUN:
		case VMHD:
		case URL:
		case URN:
			return true;

		default:
			return false;
	}
}

size_t MediaDataBox::size() const
{
	size_t size = 0;
	for ( auto b: byte_ranges ) {
		size += b.second;
	}
#ifdef _DEBUG
	if ( !chunks.empty() ) {
		size_t size_d = 0;
		for ( auto c: chunks ) {
			size_d += c.second;
		}
		assert( size == size_d );
	}
#endif
	return size;
}
