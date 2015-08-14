#include "stdafx.h"
#include "Types.h"
#include "Variables.h"

namespace SCRambl
{
	/* Variable */
	void Variable::Init() {
		
	}
	Variable::Variable(const Types::Type* type, size_t index, std::string name, size_t arrsize) : m_Name(name), m_Index(index), m_Type(type), m_ArraySize(arrsize)
	{ Init(); }
}