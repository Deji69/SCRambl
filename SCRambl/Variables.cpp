#include "stdafx.h"
#include <exception>
#include "Types.h"
#include "Variables.h"

namespace SCRambl
{
	/* Variable */
	void Variable::Init() {
		m_ID = Value()->GetVarType()->GetVarMinIndex() + m_Index;
		m_Offset = BitsToBytes(Value()->GetVarType()->Size().AsNumber<size_t>()) * m_ID;
		m_IsGlobal = Value()->GetVarType()->IsGlobal();
	}
	Variable::Variable(const Types::Type* type, size_t index, std::string name, size_t arrsize) : m_Name(name),
		m_Index(index), m_Type(type), m_ArraySize(arrsize)
	{
		if (IsArray()) m_ArrayValue = type->GetArrayValue();
		else m_VarValue = type->GetVarValue();
		Init();
	}
}