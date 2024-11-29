#include "MP4SetBox.h"
#include "MP4.h"
#include <assert.h>

// MP4SetBoxVisitor

MP4SetBox::MP4SetBoxVisitor::MP4SetBoxVisitor(const std::vector<std::string>& params)
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

MP4SetBox::MP4SetBoxVisitor::~MP4SetBoxVisitor()
{
}

void MP4SetBox::MP4SetBoxVisitor::visit(BoxHead& head, MediaDataBox& mdat)
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

// MP4SetBox

MP4SetBox::MP4SetBox(const std::vector<std::string>& params)
	: _params(params)
{
}

MP4SetBox::~MP4SetBox()
{
}

void MP4SetBox::execute(std::shared_ptr<MP4AbstractBox> mp4)
{
	assert( MP4FILE == mp4->head().boxtype );

	MP4SetBoxVisitor v(_params);
	mp4->accept(&v);
}
