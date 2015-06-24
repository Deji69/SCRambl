#include "stdafx.h"
#include "Types.h"
#include "Variables.h"

namespace SCRambl
{
	/* Variable */
	void Variable::Init() {
	}
	Variable::Variable(Types::Type* type, std::string name, size_t arrsize) : m_Name(name), m_Type(type), m_ArraySize(arrsize)
	{ Init(); }
}