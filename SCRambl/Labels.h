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
	namespace Scripts
	{
		/*\ Scripts::Label - Labels in scripts \*/
		class Label {
			std::string m_Name;
			Types::Type* m_Type;
			uint32_t m_Offset = 0;

		public:
			Label(Types::Type* type, std::string name) : m_Name(name), m_Type(type)
			{ }

			inline const std::string& GetName() const { return m_Name; }
			inline Types::Type* GetType() const { return m_Type; }
			inline uint32_t GetOffset() const { return m_Offset; }

			static inline std::string Formatter(Label* label) {
				return "(" + label->GetName() + ")";
			}
		};
	}
}
