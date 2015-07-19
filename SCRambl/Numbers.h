/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include "Lexer.h"

namespace SCRambl
{
	class Numbers {
	public:
		enum Type {
			Integer, Float, Byte, Word, DWord
		};

		class IntegerType
		{
		public:
			enum Type { None, Int, Long, Short, Char, UInt, ULong, UShort, UChar };
			
			typedef long long MaxType;
			typedef unsigned long long MaxUType;
			typedef char MinType;
			typedef unsigned char MinUType;

		private:
			Type m_Type = None;
			union Value {
				long long			tLong;
				long				tInt;
				short				tShort;
				char				tChar;
				unsigned long long	tULong;
				unsigned long		tUInt;
				unsigned short		tUShort;
				unsigned char		tUChar;
			} m_Value;
			size_t m_Size;

		public:
			IntegerType() = default;
			IntegerType(long long val) : m_Type(Long), m_Size(sizeof(long long))
			{ m_Value.tLong = val; }
			IntegerType(int val) : m_Type(Int), m_Size(sizeof(int))
			{ m_Value.tInt = val; }
			IntegerType(short val) : m_Type(Short), m_Size(sizeof(short))
			{ m_Value.tShort = val; }
			IntegerType(char val) : m_Type(Char), m_Size(sizeof(char))
			{ m_Value.tChar = val; }
			IntegerType(unsigned long long val) : m_Type(ULong), m_Size(sizeof(unsigned long long))
			{ m_Value.tULong = val; }
			IntegerType(unsigned int val) : m_Type(UInt), m_Size(sizeof(unsigned int))
			{ m_Value.tUInt = val; }
			IntegerType(unsigned short val) : m_Type(UShort), m_Size(sizeof(unsigned short))
			{ m_Value.tUShort = val; }
			IntegerType(unsigned char val) : m_Type(UChar), m_Size(sizeof(unsigned char))
			{ m_Value.tUChar = val; }
			IntegerType(const char * str) : m_Type(None), m_Size(sizeof(const char*))
			{
				if (StringToInt<long long>(str, m_Value.tLong, true) == ConvertResult::success) {
					m_Size = sizeof(long long);
					m_Type = Long;
				}
			}

			template<typename T>
			inline T GetValue() const {
				switch (m_Type) {
				case Long: return (T)m_Value.tLong;
				case Int: return (T)m_Value.tInt;
				case Short: return (T)m_Value.tShort;
				case Char: return (T)m_Value.tChar;
				case ULong: return (T)m_Value.tULong;
				case UShort: return (T)m_Value.tUShort;
				case UChar: return (T)m_Value.tUChar;
				}
				return (T)m_Value.tLong;
			}

			size_t Size() const { return m_Size; }

			inline operator long long() const { return GetValue<long long>(); }
			inline operator long() const { return GetValue<long>(); }
			inline operator short() const { return GetValue<short>(); }
			inline operator char() const { return GetValue<char>(); }
			inline operator unsigned long long() const { return GetValue<unsigned long long>(); }
			inline operator unsigned long() const { return GetValue<unsigned long>(); }
			inline operator unsigned short() const { return GetValue<unsigned short>(); }
			inline operator unsigned char() const { return GetValue<unsigned char>(); }
			inline operator size_t() const { return GetValue<size_t>(); }
		};
		class FloatType
		{
		public:
			enum Type { None, Float };

		private:
			Type			m_Type = None;
			float			m_Value;

		public:
			FloatType() = default;
			FloatType(float val) : m_Type(Float), m_Value(val)
			{ }
			FloatType(const char * str) : m_Type(None)
			{
				if (StringToFloat<float>(str, m_Value, true) == ConvertResult::success)
					m_Type = Float;
			}

			template<typename T>
			inline T GetValue() const {
				return m_Value;
			}

			inline operator float()	const { return GetValue<float>(); }
			inline size_t Size() const { return sizeof(float); }
		};

		enum class ConvertResult {
			success, not_a_number, is_a_float, is_an_int
		};

		// Handy converty functions
		template<typename TConv, typename T = TConv>
		static ConvertResult StringToInt(const char * str, T & out, bool convert_float = false) {
			long long n = 0;
			long long d = 1;		// number of decimal places
			long long f = 0;		// the RHS of the decimal point
			bool is_float = false;
			bool is_hex = false;
			bool is_neg = false;
			while (*str == '-') is_neg = !is_neg;
			if (*str == '0' && str[1] == 'x') {
				is_hex = true;
				str += 2;
			}
			bool have_one = false;
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
					else if (have_one) break;
					else return ConvertResult::not_a_number;
				}
				else {
					if (std::isdigit(*str)) {
						f = f * 10 + *str - '0';
						d *= 10;
					}
					else if (have_one) break;
					else return ConvertResult::not_a_number;
				}
				have_one = true;
			}

			if (!have_one) return ConvertResult::not_a_number;

			out = is_neg ? -(is_float ? (TConv)((n + ((float)f / (float)d))) : n)
						 : (is_float ? (TConv)((n + ((float)f / (float)d))) : n);
			return ConvertResult::success;
		}
		template<typename TConv, typename T = TConv>
		static ConvertResult StringToFloat(const char * str, T & out, bool convert_int = false) {
			long long n = 0;
			long long d = 1;		// number of decimal places
			long long f = 0;		// the RHS of the decimal point
			bool is_float = false;
			bool is_hex = false;
			bool is_neg = false;
			while (*str == '-') is_neg = !is_neg;
			if (*str == '0' && str[1] == 'x') {
				is_hex = true;
				str += 2;
			}
			bool have_one = false;
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
				have_one = true;
			}

			if (!have_one) return ConvertResult::not_a_number;

			if (!is_float) {
				if (!convert_int) return ConvertResult::is_an_int;
			}
			out = (is_float ? (TConv)(is_neg ? -((TConv)n + ((double)f / (double)d)) : ((TConv)n + ((double)f / (double)d))) : (TConv)(is_neg ? -n : n));
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

			long long		m_IntVal;
			float			m_FloatVal;

			static inline bool IsHexPrefix(char c) {
				return c == 'x' || c == 'X';
			}

		public:
			bool Scan(Lexer::State & state, Scripts::Position & pos) override {
				switch (state) {
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
					do {
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
						} else {
							if (pos->GetType() == Symbol::number) {
								f = f * 10 + *pos - '0';
								d *= 10;
							}
							else break;
						}
					} while (++pos);
					if (m_Float) m_FloatVal = n + ((float)f / (float)d);
					else m_IntVal = n;
					state = Lexer::State::after;
					return true;
				}
				case Lexer::State::after:
					return true;
				}
				return false;
			}

			template<typename T> inline T Get() const		{ return m_Float ? (T)m_FloatVal : (T)m_IntVal; }
			template<> inline float Get<float>() const		{ return m_Float ? m_FloatVal : (float)m_IntVal; }

			template<typename T> bool Is() const;
			template<> inline bool Is<int>() const			{ return !m_Float; }
			template<> inline bool Is<float>() const		{ return m_Float; }
		};
	};
}