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
	public:
		enum Error {
			error_invalid_hex_char,
			error_invalid_decimal_char,
		};

	private:
		bool m_Float;
		bool m_Hex;
		
		union {
			int		m_IntVal;
			float	m_FloatVal;
		};

		int NumToInt(char c) const {
			if (c >= '0' && c <= '9') return c - '0';

			if (m_Hex)
			{
				c = std::tolower(c);
				
				if (c >= 'a' && c <= 'f') return c - 'a' + 0x10;
				throw(error_invalid_hex_char);
			}
			
			throw(error_invalid_decimal_char);
		}
		inline bool IsDigitValid(char c) const {
			if (c >= '0' && c <= '9') return true;
			return m_Hex && ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
		}
		static inline bool IsHexPrefix(char c) {
			return c == 'x' || c == 'X';
		}

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
					// if 0 is first, check for a prefix
					if (*pos == '0')
					{
						auto pre = pos;
						if (++pre)
						{
							if (IsHexPrefix(*pre))
							{
								m_Hex = true;
								pos = pre;
								if (!++pos) return false;
							}
						}
					}
					state = Lexer::State::inside;
					return true;
				}
				return false;

			case Lexer::State::inside:
			{
				int n = 0;
				do
				{
					// make numbers?
					if (pos->GetType() == Symbol::number)
						n = n * (m_Hex ? 0x10 : 10) + *pos - '0';
					else if (m_Hex)
					{
						if (*pos >= 'A' && *pos <= 'F')
							n = n * 0x10 + *pos - 'A' + 0xA;
						else if (*pos >= 'a' && *pos <= 'f')
							n = n * 0x10 + *pos - 'a' + 0xA;
						else break;
					}
					else break;
				}
				while (++pos);
				m_IntVal = n;
				state = Lexer::State::after;
				return true;
			}

			case Lexer::State::after:
				return true;
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