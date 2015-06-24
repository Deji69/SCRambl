/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include <vector>
#include "Scripts-code.h"
#include "Types.h"

namespace SCRambl
{
	/*\ Label - Labels in scripts \*/
	class Label {
		std::string m_Name;
		Types::Type* m_Type;
		Scripts::Position m_Position;

	public:
		Label(Types::Type* type, std::string name, Scripts::Position pos);

		inline std::string Name() const { return m_Name; }
		inline Types::Type* Type() const { return m_Type; }
		inline Scripts::Position Pos() const { return m_Position; }

		static std::string Formatter(Label* label);
	};
}
