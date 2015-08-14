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
		const Types::Type* m_Type;
		size_t m_Index;
		size_t m_ArraySize = 0;
		bool m_IsGlobal = false;

		void Init();

	public:
		Variable(const Types::Type* type, size_t index, std::string name, size_t array_size = 0);

		inline size_t Index() const { return m_Index; }
		inline std::string Name() const { return m_Name; }
		inline const Types::Type* Type() const { return m_Type; }
		inline Types::VariableValue* Value() const { return IsArray() ? m_Type->GetArrayValue() : m_Type->GetVarValue(); }
		inline Types::VariableValue* ArrayValue() const {
			ASSERT(IsArray());
			return m_Type->GetArrayValue();
		}
		inline bool IsArray() const { return m_ArraySize != 0; }
		inline bool IsGlobal() const { return m_IsGlobal; }
	};
}