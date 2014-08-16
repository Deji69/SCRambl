/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>

namespace SCRambl
{
	class Label
	{
		std::string			m_Name;

	public:
		using Shared = std::shared_ptr < Label > ;

		Label(std::string name) : m_Name(name)
		{ }
	};
}
