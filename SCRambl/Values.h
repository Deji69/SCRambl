/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include "utils.h"
#include "Configuration.h"
#include "Attributes.h"
#include "Tokens.h"
#include "TokensB.h"
#include "Types.h"

namespace SCRambl
{
	class Build;

	class Value
	{
	public:
		Value(const Types::Type& type) : m_Type(type)
		{ }
		inline virtual ~Value() { }

		virtual size_t GetMinimumSize() const = 0;

		inline const Types::Type& GetType() const { return m_Type; }

	private:
		const Types::Type& m_Type;
	};
}