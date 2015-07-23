/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include "Symbols.h"
#include "Lexer.h"

namespace SCRambl
{
	/*\
	 - StringScanner for lexing string literals
	\*/
	class StringScanner : public Lexing::Scanner
	{
	public:
		bool Scan(Lexing::State & state, Scripts::Position & pos) override
		{
			switch (state)
			{
			case Lexing::State::before:
				if (*pos == '"') {
					++pos;
					state = Lexing::State::inside;
					return true;
				}
				return false;
			case Lexing::State::inside:
				for (bool escape = false; pos; ++pos)
				{
					if (pos->GetType() == Symbol::eol && !escape)
					{
					}
					else if (*pos == '"')
					{
						++pos;
						break;
					}
					else if (*pos == '\\') escape = !escape;
					else escape = false;
				}
				return true;
			}
			return false;
		}
	};
}