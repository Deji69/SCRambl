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
		bool m_Float;
		bool m_Hex;
		
		union {
			int		m_IntVal;
			float	m_FloatVal;
		};

	public:
		bool Scan(Lexer::State & state, Script::Position & code) override
		{
			switch (state)
			{
			case Lexer::State::before:
				m_Hex = false;
				m_Float = false;
				return false;

			case Lexer::State::inside:
				return false;

			case Lexer::State::after:
				return false;
			}
			return false;
		}

		template<typename V>
		V GetValue() const;

		template<>
		inline int GetValue<int>() const		{ return m_IntVal; }
		template<>
		inline float GetValue<float>() const	{ return m_FloatVal; }
	};
}