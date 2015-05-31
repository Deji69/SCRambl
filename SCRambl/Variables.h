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
		Types::Type* m_Type;
		Types::Variable* m_VarType;
		size_t m_ArraySize = 0;
		bool m_IsGlobal = false;

		void Init() {
			m_VarType = m_Type->ToVariable();
			m_IsGlobal = m_VarType->IsGlobal();
			//if (m_VarType->)
		}

	public:
		Variable(std::string name, Types::Type* type, size_t array_size = 0) : m_Name(name), m_Type(type), m_ArraySize(array_size)
		{ Init(); }

		inline const std::string& Name() const { return m_Name; }
		inline Types::Type* Type() const { return m_Type; }
		inline bool IsArray() const { return m_ArraySize != 0; }
		inline bool IsGlobal() const { return m_IsGlobal; }
	};
}