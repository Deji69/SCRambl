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

	public:
		using Shared = std::shared_ptr<Variable>;

		Variable(std::string name, Types::Type::Shared type) : m_Name(name), m_Type(type)
		{ }

		inline const std::string& Name() const { return m_Name; }
		inline Types::Type::Shared Type() const { return m_Type; }
	};
}