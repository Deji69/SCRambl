#include "stdafx.h"
#include <exception>
#include "Types.h"
#include "Variables.h"

namespace SCRambl
{
	/* Variable */
	void Variable::Init() {
		m_ID = Value()->GetVarType()->GetVarMinIndex() + m_Index;
	}
	Variable::Variable(const Types::Type* type, size_t index, std::string name, size_t arrsize) : m_Name(name),
		m_Index(index), m_Type(type), m_ArraySize(arrsize)
	{ Init(); }
}