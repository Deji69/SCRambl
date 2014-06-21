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
	class Token
	{
	public:
		enum Type
		{
			None,
			Whitespace,
			Eol,
			Identifier,
			Directive,
			OpenParen,
			CloseParen,
			Comment,
			BlockComment,
			Number,
			Operator,
			String,
			Invalid,
			Max = Invalid
		};

	private:
		Type				m_Type;

	};

	template<typename T>
	class Tokens
	{
		
	};
}