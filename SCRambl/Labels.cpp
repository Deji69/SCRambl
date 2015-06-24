#include "stdafx.h"
#include "Labels.h"

namespace SCRambl
{
	/* Label */
	Label::Label(Types::Type* type, std::string name, Scripts::Position pos) : m_Name(name), m_Type(type), m_Position(pos)
	{ }
	std::string Label::Formatter(Label* label) {
		return "(" + label->Name() + ")";
	}
}