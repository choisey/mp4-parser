/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4.h"
#include <stdarg.h>

std::vector<std::shared_ptr<MP4AbstractBox>> select(std::shared_ptr<MP4AbstractBox> b, uint32_t t)
{
	std::vector<std::shared_ptr<MP4AbstractBox>> v;

	if ( b->head().boxtype == t ) {
		v.push_back( b );
	}

	b->select(t, v);

	return v;
}

#ifdef _DEBUG

std::ostream& operator<<(std::ostream& os, std::shared_ptr<MP4AbstractBox> box)
{
	os << '(' << box->head().offset << ',' << box->head().boxsize << ')';
	return os;
}

std::ostream& operator<<(std::ostream& os, const std::pair<uint32_t, uint32_t> data)
{
	return mp4_debug_dump_pair<uint32_t, uint32_t>(os, data);
	//os << '(' << data.first << ',' << data.second << ')';
	//return os;
}

std::ostream& operator<<(std::ostream& os, const std::pair<uint64_t, uint32_t> data)
{
	return mp4_debug_dump_pair<uint64_t, uint32_t>(os, data);
}

std::ostream& operator<<(std::ostream& os, const std::pair<uint64_t, uint64_t> data)
{
	return mp4_debug_dump_pair<uint64_t, uint64_t>(os, data);
	//os << '(' << data.first << ',' << data.second << ')';
	//return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<uint32_t>& data)
{
	return mp4_debug_dump_vector<uint32_t>(os, data);
//	os << '{';
//	int count = 0;
//	for ( auto e: data ) {
//		if ( count++ ) os << ',';
//		if ( DUMP_MAX_ENTRIES < count ) {
//			//os << " ...";
//			os << " ... , " << data.back();
//			break;
//		}
//		os << ' ' << e;
//	}
//	os << " }";
//	return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<uint64_t>& data)
{
	return mp4_debug_dump_vector<uint64_t>(os, data);
//	os << '{';
//	int count = 0;
//	for ( auto e: data ) {
//		if ( count++ ) os << ',';
//		if ( DUMP_MAX_ENTRIES < count ) {
//			//os << " ...";
//			os << " ... , " << data.back();
//			break;
//		}
//		os << ' ' << e;
//	}
//	os << " }";
//	return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<std::pair<uint64_t, uint64_t>>& data)
{
	return mp4_debug_dump_vector<std::pair<uint64_t, uint64_t>>(os, data);
//	os << '{';
//	int count = 0;
//	for ( auto e: data ) {
//		if ( count++ ) os << ',';
//		if ( DUMP_MAX_ENTRIES < count ) {
//			//os << " ... , (" << data.back().first << ',' << data.back().second << ')';
//			os << " ... , " << data.back();
//			break;
//		}
//		os << " (" << e.first << ',' << e.second << ')';
//	}
//	os << " }";
//	return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<std::shared_ptr<MP4AbstractBox>>& data)
{
	return mp4_debug_dump_vector<std::shared_ptr<MP4AbstractBox>>(os, data);
//	os << '{';
//	int count = 0;
//	for ( auto e: data ) {
//		if ( count++ ) os << ',';
//		if ( DUMP_MAX_ENTRIES < count ) {
//			//os << " ... , (" << data.back()->head().offset << ',' << data.back()->head().boxsize << ')';
//			os << " ... , (" << data.back();
//			break;
//		}
//		os << " (" << e->head().offset << ", " << e->head().boxsize << ')';
//	}
//	os << " }";
//	return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<uint32_t>>& data)
{
	return mp4_debug_dump_vector<std::vector<uint32_t>>(os, data);
//	os << '{';
//	int c1 = 0;
//	for ( auto e1: data ) {
//		if ( c1++ ) os << ',';
//		if ( DUMP_MAX_ENTRIES < c1 ) {
//			//os << " ...";
//			os << " ... , ";
//			os << ' ' << '(';
//			int c2 = 0;
//			for ( auto e2: e1 ) {
//				if ( c2++ ) os << ',';
//				os << e2;
//			}
//			os << ')';
//			break;
//		}
//		os << ' ' << '(';
//		int c2 = 0;
//		for ( auto e2: e1 ) {
//			if ( c2++ ) os << ',';
//			os << e2;
//		}
//		os << ')';
//	}
//	os << " }";
//	return os;
}

std::ostream& operator<<(std::ostream& os, const std::map<uint32_t, uint32_t>& data)
{
	return mp4_debug_dump_map<uint32_t, uint32_t>(os, data);
//	os << '{';
//	int count = 0;
//	for ( auto e: data ) {
//		if ( count++ ) os << ',';
//		if ( DUMP_MAX_ENTRIES < count ) {
//			os << " ... , " << data.rbegin()->first << "->" << data.rbegin()->second;
//			break;
//		}
//		os << ' ' << e.first << "->" << e.second;
//	}
//	os << " }";
//	return os;
}

std::ostream& operator<<(std::ostream& os, const std::map<uint32_t, uint64_t>& data)
{
	return mp4_debug_dump_map<uint32_t, uint64_t>(os, data);
//	os << '{';
//	int count = 0;
//	for ( auto e: data ) {
//		if ( count++ ) os << ',';
//		if ( DUMP_MAX_ENTRIES < count ) {
//			os << " ... , " << data.rbegin()->first << "->" << data.rbegin()->second;
//			break;
//		}
//		os << ' ' << e.first << "->" << e.second;
//	}
//	os << " }";
//	return os;
}

std::ostream& operator<<(std::ostream& os, const std::map<uint64_t, uint32_t>& data)
{
	return mp4_debug_dump_map<uint64_t, uint32_t>(os, data);
//	os << '{';
//	int count = 0;
//	for ( auto e: data ) {
//		if ( count++ ) os << ',';
//		if ( DUMP_MAX_ENTRIES < count ) {
//			//os << " ...";
//			os << " ... , " << data.rbegin()->first << "->" << data.rbegin()->second;
//			break;
//		}
//		os << ' ' << e.first << "->" << e.second;
//	}
//	os << " }";
//	return os;
}

std::ostream& operator<<(std::ostream& os, const std::map<uint64_t, uint64_t>& data)
{
	return mp4_debug_dump_map<uint64_t, uint64_t>(os, data);
//	os << '{';
//	int count = 0;
//	for ( auto e: data ) {
//		if ( count++ ) os << ',';
//		if ( DUMP_MAX_ENTRIES < count ) {
//			//os << " ...";
//			os << " ... , " << data.rbegin()->first << "->" << data.rbegin()->second;
//			break;
//		}
//		os << ' ' << e.first << "->" << e.second;
//	}
//	os << " }";
//	return os;
}

std::ostream& operator<<(std::ostream& os, const std::map<uint64_t, std::pair<uint32_t, uint32_t>>& data)
{
	return mp4_debug_dump_map<uint64_t, std::pair<uint32_t, uint32_t>>(os, data);
//	os << '{';
//	int count = 0;
//	for ( auto e: data ) {
//		if ( count++ ) os << ',';
//		if ( DUMP_MAX_ENTRIES < count ) {
//			//os << " ...";
//			//os << " ... , " << data.rbegin()->first << "->(" << data.rbegin()->second.first << ',' << data.rbegin()->second.second << ')';
//			os << " ... , " << data.rbegin()->first << "->" << data.rbegin()->second;
//			break;
//		}
//		os << ' ' << e.first << "->(" << e.second.first << ',' << e.second.second << ')';
//	}
//	os << " }";
//	return os;
}

std::ostream& operator<<(std::ostream& os, const std::map<uint64_t, std::pair<uint64_t, uint32_t>>& data)
{
	return mp4_debug_dump_map<uint64_t, std::pair<uint64_t, uint32_t>>(os, data);
//	os << '{';
//	int count = 0;
//	for ( auto e: data ) {
//		if ( count++ ) os << ',';
//		if ( DUMP_MAX_ENTRIES < count ) {
//			//os << " ...";
//			//os << " ... , " << data.rbegin()->first << "->(" << data.rbegin()->second.first << ',' << data.rbegin()->second.second << ')';
//			os << " ... , " << data.rbegin()->first << "->" << data.rbegin()->second;
//			break;
//		}
//		os << ' ' << e.first << "->(" << e.second.first << ',' << e.second.second << ')';
//	}
//	os << " }";
//	return os;
}

std::ostream& operator<<(std::ostream& os, const std::map<uint32_t, std::vector<uint64_t>>& data)
{
	return mp4_debug_dump_map<uint32_t, std::vector<uint64_t>>(os, data);
}

std::ostream& operator<<(std::ostream& os, const std::map<uint64_t, std::vector<uint64_t>>& data)
{
	return mp4_debug_dump_map<uint64_t, std::vector<uint64_t>>(os, data);
}

#endif
