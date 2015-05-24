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
		Types::Type::Shared m_Type;
		size_t m_ArraySize = 0;

	public:
		using Shared = std::shared_ptr<Variable>;

		Variable(std::string name, Types::Type::Shared type, size_t array_size = 0) : m_Name(name), m_Type(type), m_ArraySize(array_size)
		{ }

		inline const std::string& Name() const { return m_Name; }
		inline Types::Type::Shared Type() const { return m_Type; }

		static Shared MakeShared(std::string name, Types::Type::Shared type, size_t array_size) {
			return std::make_shared<Variable>(name, type, array_size);
		}
	};
}