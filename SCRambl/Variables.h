/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <memory>
#include "Types.h"

namespace SCRambl
{
	class Variable
	{
		std::string	m_Name;
		Types::VarType m_VarType;
		Types::Type* m_Type;
		size_t m_ArraySize = 0;
		bool m_IsGlobal = false;

		void Init();

	public:
		Variable(Types::Type* type, std::string name, size_t array_size = 0);

		inline std::string Name() const { return m_Name; }
		inline Types::Type* Type() const { return m_Type; }
		inline Types::VariableValue* Value() const { return IsArray() ? m_Type->GetArrayValue() : m_Type->GetVarValue(); }
		inline Types::VariableValue* ArrayValue() const {
			ASSERT(IsArray());
			return m_Type->GetArrayValue();
		}
		inline bool IsArray() const { return m_ArraySize != 0; }
		inline bool IsGlobal() const { return Value()->GetType()->IsGlobal(); }
	};
}