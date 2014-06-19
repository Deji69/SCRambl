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
		bool Scan(Lexer::State & state, Script::Position & pos) override
		{
			switch (state)
			{
			case Lexer::State::before:
				m_Hex = false;
				m_Float = false;

				// obviously we need to make sure this is a number
				if (pos->GetType() == Symbol::number)
				{
					++pos;
				}
				return false;

			case Lexer::State::inside:
				return false;

			case Lexer::State::after:
				return false;
			}
			return false;
		}

		template<typename T>
		inline T Get() const					{ return m_Float ? m_FloatVal : m_IntVal; }

		template<typename T> bool Is() const;

		template<>
		inline bool Is<int>() const				{ return !m_Float; }
		template<>
		inline bool Is<float>() const			{ return m_Float; }
	};
}