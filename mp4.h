/*
 * Copyright (c) Seungyeob Choi
 */

#pragma once

#include "mp4_container_box.h"
#include "mp4_concrete_box.h"
#include <typeinfo>
#include <iostream>

//#define MAX_32BIT	0xFFFFFFFF
//#define MAX_64BIT	0xFFFFFFFFFFFFFFFF
static const uint32_t MAX_32BIT	= 0xFFFFFFFF;
static const uint64_t MAX_64BIT	= 0xFFFFFFFFFFFFFFFF;

std::vector<std::shared_ptr<mp4_abstract_box>> select(
		std::shared_ptr<mp4_abstract_box>,
		uint32_t);

template<typename T>
std::vector<std::shared_ptr<mp4_concrete_box<T>>> select(std::shared_ptr<mp4_abstract_box> b) {
	std::vector<std::shared_ptr<mp4_abstract_box>> v;

	if ( b->istype( typeid(T) ) ) {
		v.push_back( b );
	}

	b->select(typeid(T), v);

	std::vector<std::shared_ptr<mp4_concrete_box<T>>> vc;
	for ( auto iter = v.begin(); iter != v.end(); iter++ ) {
		vc.push_back( std::static_pointer_cast<mp4_concrete_box<T>>( *iter ) );
	}

	return vc;
}

#ifdef _DEBUG
#define DUMP_MAX_ENTRIES 100

template <typename T1, typename T2>
std::ostream& mp4_debug_dump_pair(std::ostream& os, const std::pair<T1, T2> data)
{
	os << '(' << data.first << ',' << data.second << ')';
	return os;
}

template <typename T>
std::ostream& mp4_debug_dump_vector(std::ostream& os, const std::vector<T>& data)
{
	os << '{';
	int count = 0;
	for ( auto e: data ) {
		if ( count++ ) os << ',';
		if ( DUMP_MAX_ENTRIES < count ) {
			//os << " ...";
			os << " ... , " << data.back();
			break;
		}
		os << ' ' << e;
	}
	os << " }";
	return os;
}

template <typename T1, typename T2>
std::ostream& mp4_debug_dump_map(std::ostream& os, const std::map<T1, T2>& data)
{
	os << '{';
	int count = 0;
	for ( auto e: data ) {
		if ( count++ ) os << ',';
		if ( DUMP_MAX_ENTRIES < count ) {
			os << " ... , " << data.rbegin()->first << "->" << data.rbegin()->second;
			break;
		}
		os << ' ' << e.first << "->" << e.second;
	}
	os << " }";
	return os;
}

std::ostream& operator<<(std::ostream&, std::shared_ptr<mp4_abstract_box>);
std::ostream& operator<<(std::ostream&, const std::pair<uint32_t, uint32_t>);
std::ostream& operator<<(std::ostream&, const std::pair<uint64_t, uint32_t>);
std::ostream& operator<<(std::ostream&, const std::pair<uint64_t, uint64_t>);
std::ostream& operator<<(std::ostream&, const std::vector<uint32_t>&);
std::ostream& operator<<(std::ostream&, const std::vector<uint64_t>&);
std::ostream& operator<<(std::ostream&, const std::vector<std::pair<uint64_t, uint64_t>>&);
std::ostream& operator<<(std::ostream&, const std::vector<std::shared_ptr<mp4_abstract_box>>&);
std::ostream& operator<<(std::ostream&, const std::vector<std::vector<uint32_t>>&);
std::ostream& operator<<(std::ostream&, const std::map<uint32_t, uint32_t>&);
std::ostream& operator<<(std::ostream&, const std::map<uint32_t, uint64_t>&);
std::ostream& operator<<(std::ostream&, const std::map<uint64_t, uint32_t>&);
std::ostream& operator<<(std::ostream&, const std::map<uint64_t, uint64_t>&);
std::ostream& operator<<(std::ostream&, const std::map<uint64_t, std::pair<uint32_t, uint32_t>>&);
std::ostream& operator<<(std::ostream&, const std::map<uint64_t, std::pair<uint64_t, uint32_t>>&);
std::ostream& operator<<(std::ostream&, const std::map<uint32_t, std::vector<uint64_t>>&);
std::ostream& operator<<(std::ostream&, const std::map<uint64_t, std::vector<uint64_t>>&);
#endif
