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
		size_t m_Index;
		size_t m_Offset;
		std::string m_Name;
		bool m_Global = false;
		const Types::Type* m_Type;

	public:
		Label(const Types::Type* type, size_t index, std::string name, size_t offset, bool global = false);

		inline size_t Index() const { return m_Index; }
		inline size_t Offset() const { return m_Offset; }
		inline std::string Name() const { return m_Name; }
		inline const Types::Type* Type() const { return m_Type; }
		inline bool IsGlobal() const { return m_Global; }
		inline void SetOffset(size_t offset) { m_Offset = offset; }

		static std::string Formatter(Label* label);
	};
}
