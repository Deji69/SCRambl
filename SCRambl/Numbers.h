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
	class Numbers {
	public:
		enum Type {
			Integer, Float, Byte, Word, DWord
		};

		typedef int IntegerType;
		typedef float FloatType;

		enum class ConvertResult {
			success, not_a_number, is_a_float, is_an_int
		};

		// Handy converty functions
		static ConvertResult StringToInt(const char * str, IntegerType & out, bool convert_float = false) {
			unsigned long n = 0;
			unsigned long d = 1;		// number of decimal places
			unsigned long f = 0;		// the RHS of the decimal point
			bool is_float = false;
			bool is_hex = false;
			for (; *str; ++str)
			{
				// make numbers, not war?
				if (!is_float) {
					if (std::isdigit(*str)) n = n * (is_hex ? 0x10 : 10) + *str - '0';
					else if (is_hex) {
						if (*str >= 'A' && *str <= 'F') n = n * 0x10 + *str - 'A' + 0xA;
						else if (*str >= 'a' && *str <= 'f') n = n * 0x10 + *str - 'a' + 0xA;
						else break;
					}
					else if (*str == '.') {
						if (!convert_float) return ConvertResult::is_a_float;
						is_float = true;
					}
					else return ConvertResult::not_a_number;
				}
				else {
					if (std::isdigit(*str)) {
						f = f * 10 + *str - '0';
						d *= 10;
					}
					else return ConvertResult::not_a_number;
				}
			}

			out = is_float ? (IntegerType)((n + ((FloatType)f / (FloatType)d))) : n;
			return ConvertResult::success;
		}
		static ConvertResult StringToFloat(const char * str, FloatType & out, bool convert_int = false) {
			unsigned long n = 0;
			unsigned long d = 1;		// number of decimal places
			unsigned long f = 0;		// the RHS of the decimal point
			bool is_float = false;
			bool is_hex = false;
			for (; *str; ++str)
			{
				// make numbers, not war?
				if (!is_float) {
					if (std::isdigit(*str)) n = n * (is_hex ? 0x10 : 10) + *str - '0';
					else if (is_hex) {
						if (*str >= 'A' && *str <= 'F') n = n * 0x10 + *str - 'A' + 0xA;
						else if (*str >= 'a' && *str <= 'f') n = n * 0x10 + *str - 'a' + 0xA;
						else break;
					}
					else if (*str == '.') {
						is_float = true;
					}
					else return ConvertResult::not_a_number;
				}
				else {
					if (std::isdigit(*str)) {
						f = f * 10 + *str - '0';
						d *= 10;
					}
					else return ConvertResult::not_a_number;
				}
			}

			if (!is_float) {
				if (!convert_int) return ConvertResult::is_an_int;
			}
			out = is_float ? (n + ((FloatType)f / (FloatType)d)) : n;
			return ConvertResult::success;
		}

		class Scanner : public Lexer::Scanner
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
				IntegerType		m_IntVal;
				FloatType		m_FloatVal;
			};

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
					if (pos->GetType() == Symbol::number) {
						// if 0 is first, check for a prefix
						if (*pos == '0') {
							auto pre = pos;
							if (++pre) {
								if (IsHexPrefix(*pre)) {
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

				case Lexer::State::inside: {
					// avoid much use of floats
					unsigned long n = 0;
					unsigned long d = 1;		// number of decimal places
					unsigned long f = 0;		// the RHS of the decimal point
					do
					{
						// make numbers, not war?
						if (!m_Float) {
							if (pos->GetType() == Symbol::number)
								n = n * (m_Hex ? 0x10 : 10) + *pos - '0';
							else if (m_Hex) {
								if (*pos >= 'A' && *pos <= 'F')
									n = n * 0x10 + *pos - 'A' + 0xA;
								else if (*pos >= 'a' && *pos <= 'f')
									n = n * 0x10 + *pos - 'a' + 0xA;
								else break;
							}
							else if (*pos == '.') {
								// lets start floating
								m_Float = true;
							}
							else break;
						}
						else {
							if (pos->GetType() == Symbol::number) {
								f = f * 10 + *pos - '0';
								d *= 10;
							}
							else break;
						}
					} while (++pos);
					if (m_Float) {
						m_FloatVal = n + ((FloatType)f / (FloatType)d);
					}
					else m_IntVal = n;
					state = Lexer::State::after;
					return true;
				}

				case Lexer::State::after:
					return true;
				}
				return false;
			}

			template<typename T> inline T Get() const;
			template<> inline int Get<IntegerType>() const		{ return m_Float ? (IntegerType)m_FloatVal : m_IntVal; }
			template<> inline float Get<FloatType>() const		{ return m_Float ? m_FloatVal : (FloatType)m_IntVal; }

			template<typename T> bool Is() const;
			template<> inline bool Is<IntegerType>() const		{ return !m_Float; }
			template<> inline bool Is<FloatType>() const		{ return m_Float; }
		};
	};
}