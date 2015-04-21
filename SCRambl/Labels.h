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

namespace SCRambl
{
	namespace Scripts
	{
		/*\ Scripts::Label - Labels in scripts \*/
		class Label
		{
			std::string				m_Name;
			uint32_t				m_Offset = 0;

		public:
			using Shared = std::shared_ptr < Label >;

			Label(std::string name) : m_Name(name)
			{ }

			inline const std::string &			GetName() const			{ return m_Name; }
			inline uint32_t						GetOffset() const		{ return m_Offset; }

			static inline Shared Make(std::string name) {
				return std::make_shared<Label>(name);
			}

			static inline std::string Formatter(Shared label) {
				return "(" + label->GetName() + ")";
			}
		};
	}
}
