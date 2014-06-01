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
	class StringScanner : public Lexer::Scanner
	{
	public:
		bool Scan(Lexer::State & state, Script::Position & pos) override
		{
			switch (state)
			{
			case Lexer::State::before:
				if (*pos == '"')
				{
					++pos;
					state = Lexer::State::inside;
					return true;
				}
				return false;
			case Lexer::State::inside:
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

	// Integer constant type
	class IntConst
	{
		long long m_nVal;

		// amount of parsed characters in integer constant (index of first invalid character)
		size_t			m_nSize;

	public:
		IntConst(const std::string &);

		template<class T>
		inline T		Value() const				{ return m_nVal; }
		inline size_t	Pos() const					{ return m_nSize; }
		inline bool		IsSigned() const			{ return m_nVal < 0; }
	};

	// Floating-point constant type
	class FloatConst
	{
		double m_fVal;

		// amount of parsed characters in floating-point constant (index of first invalid character)
		size_t			m_nSize;

	public:
		FloatConst(const std::string &);

		template<class T>
		inline T		Value() const				{ return m_fVal; }
		inline size_t	Pos() const					{ return m_nSize; }
	};

	// Character literal
	class CharLiteral
	{
		int		m_nVal;
		size_t	m_nSize;

	public:
		CharLiteral(const std::string &);

		inline int	Value() const					{ return m_nVal; }
		inline size_t Pos() const					{ return m_nSize; }
	};

	// String literal
}