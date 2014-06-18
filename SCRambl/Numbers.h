/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Scripts.h"
#include "Lexer.h"

namespace SCRambl
{
	class NumericScanner : public Lexer::Scanner
	{
		bool Float;
		bool Hex;
		int Int;

	public:
		bool Scan(Lexer::State & state, Script::Position & code) override
		{
			switch (state)
			{
			case Lexer::State::before:
				Int = 0;
				return false;

			case Lexer::State::inside:
				return false;

			case Lexer::State::after:
				return false;
			}
			return false;
		}

		inline int GetInt() const		{ return Int; }
	};
}