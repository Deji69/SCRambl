#include "stdafx.h"
#include "Labels.h"

namespace SCRambl
{
	/* Label */
	Label::Label(const Types::Type* type, size_t index, std::string name, size_t offset, bool global) : m_Name(name), m_Type(type), m_Index(index), m_Offset(offset), m_Global(global)
	{ }
	std::string Label::Formatter(Label* label) {
		return "(" + label->Name() + ")";
	}
}