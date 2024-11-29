/*
 * Copyright (c) Seungyeob Choi
 */

#include "mp4_set_box.h"
#include "mp4.h"
#include <assert.h>

// mp4_set_box_visitor

mp4_set_box::mp4_set_box_visitor::mp4_set_box_visitor(const std::vector<std::string>& params)
{
	for ( auto& param: params ) {
		size_t eq = param.find_first_of('=');
		std::string identifier = param.substr(0, eq);
		std::string val = param.substr(eq + 1);

		size_t co = identifier.find_first_of(':');
		std::string box = identifier.substr(0, co);
		std::string field = identifier.substr(co + 1);

		auto bi = _substitutions.find(box);
		if ( _substitutions.end() != bi ) {
			bi->second.push_back(std::make_pair(field, val));
		}
		else {
			std::vector<std::pair<std::string, std::string>> sub;
			sub.push_back(std::make_pair(field, val));
			_substitutions[box] = sub;
		}
	}
}

mp4_set_box::mp4_set_box_visitor::~mp4_set_box_visitor()
{
}

void mp4_set_box::mp4_set_box_visitor::visit(BoxHead& head, MediaDataBox& mdat)
{
	auto si = _substitutions.find("mdat");
	if ( _substitutions.end() != si ) {
		for ( auto& param: si->second ) {
			std::string& field = param.first;
			std::string& value = param.second;
			if ( 0 == field.compare("uri") ) {
				mdat.uri = value;
			}
		}
	}
}

// mp4_set_box

mp4_set_box::mp4_set_box(const std::vector<std::string>& params)
	: _params(params)
{
}

mp4_set_box::~mp4_set_box()
{
}

void mp4_set_box::execute(std::shared_ptr<mp4_abstract_box> mp4)
{
	assert( MP4FILE == mp4->head().boxtype );

	mp4_set_box_visitor v(_params);
	mp4->accept(&v);
}
