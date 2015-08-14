#include "stdafx.h"
#include "Labels.h"

namespace SCRambl
{
	/* Label */
	Label::Label(const Types::Type* type, size_t index, std::string name, Scripts::Position pos) : m_Name(name), m_Type(type), m_Position(pos), m_Index(index)
	{ }
	std::string Label::Formatter(Label* label) {
		return "(" + label->Name() + ")";
	}
}