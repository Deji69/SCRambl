/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include "utils.h"
#include "Lexer.h"

namespace SCRambl
{
	class Numbers {
	public:
		enum Type {
			Integer, Float, Byte, Word, DWord
		};
		enum class ConvertResult {
			success, not_a_number, is_a_float, is_an_int
		};

		class IntegerType
		{
		public:
			enum Type { None, Int, Long, Short, Char, UInt, ULong, UShort, UChar };
			
			typedef long long MaxType;
			typedef unsigned long long MaxUType;
			typedef char MinType;
			typedef unsigned char MinUType;

		public:
			IntegerType() = default;
			IntegerType(long long val) {
				TypeSetup(CountBitOccupation(val), true);
				SetValue(val);
			}
			IntegerType(unsigned long long val) {
				TypeSetup(CountBitOccupation(val), false);
				SetValue(val);
			}
			IntegerType(const char * str) : m_Type(None), m_Size(sizeof(const char*))
			{
				if (StringToInt<long long>(str, m_Value.tLong, true) == ConvertResult::success) {
					SetType(Long);
				}
				else BREAK();
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

			inline size_t Size() const { return m_Size; }
			inline Type GetType() const { return m_Type; }
			inline bool IsSigned() const { return IsSignedType(GetType()); }
			inline void Sign(bool negate = false) {
				TypeSetup(CountBitOccupation(m_Value.tULong), false);
				m_Type = GetSignedType(m_Type);

				// make the number negatively negative, assuming it's positively positive
				if (negate && m_Value.tULong > 0) {
					SetValue(-static_cast<long long>(m_Value.tULong));
					m_Size = CountBitOccupation(m_Value.tULong);
				}
			}

			inline operator long long() const { return GetValue<long long>(); }
			inline operator long() const { return GetValue<long>(); }
			inline operator short() const { return GetValue<short>(); }
			inline operator char() const { return GetValue<char>(); }
			inline operator unsigned long long() const { return GetValue<unsigned long long>(); }
			inline operator unsigned long() const { return GetValue<unsigned long>(); }
			inline operator unsigned short() const { return GetValue<unsigned short>(); }
			inline operator unsigned char() const { return GetValue<unsigned char>(); }
			inline operator size_t() const { return GetValue<size_t>(); }

			static Type GetSignedType(Type type) {
				switch (type) {
				case ULong: return Long;
				case UInt: return Int;
				case UShort: return Short;
				case UChar: return Char;
				}
				return type;
			}
			static Type GetUnsignedType(Type type) {
				switch (type) {
				case Long: return ULong;
				case Int: return UInt;
				case Short: return UShort;
				case Char: return UChar;
				}
				return type;
			}

		private:
			inline void TypeSetup(size_t size, bool sign) {
				if (sign ? (size > 32) : (size > 31)) SetType(sign ? Long : ULong);
				else if (sign ? (size > 16) : (size > 15)) SetType(sign ? Int : UInt);
				else if (sign ? (size > 8) : (size > 7)) SetType(sign ? Short : UShort);
				else SetType(sign ? Char : UChar);
			}
			void SetValue(long long val) {
				if (val < 0) {
					switch (m_Type) {
					case Type::Char:
						m_Value.tChar = val;
						break;
					case Type::Short:
						m_Value.tShort = val;
						break;
					case Type::Int:
						m_Value.tInt = val;
						break;
					case Type::Long:
						m_Value.tLong = val;
						break;
					}
				}
				else m_Value.tULong = val;
			}
			void SetType(Type type) {
				m_Type = type;
				switch (type) {
				case Type::Char:
					m_Size = BytesToBits(sizeof(char));
					break;
				case Type::UChar:
					m_Size = BytesToBits(sizeof(unsigned char));
					break;
				case Type::Short:
					m_Size = BytesToBits(sizeof(short));
					break;
				case Type::UShort:
					m_Size = BytesToBits(sizeof(unsigned short));
					break;
				case Type::Int:
					m_Size = BytesToBits(sizeof(int));
					break;
				case Type::UInt:
					m_Size = BytesToBits(sizeof(unsigned int));
					break;
				case Type::Long:
					m_Size = BytesToBits(sizeof(long long));
					break;
				case Type::ULong:
					m_Size = BytesToBits(sizeof(unsigned long long));
					break;
				}
			}
			
			static bool IsSignedType(Type type) {
				switch (type) {
				case Char:
				case Short:
				case Int:
				case Long:
					return true;
				case UChar:
				case UShort:
				case UInt:
				case ULong:
					break;
				default: BREAK();
				}
				return false;
			}

		private:
			Type m_Type = None;
			union Value {
				long long			tLong = 0;
				long				tInt;
				short				tShort;
				char				tChar;
				unsigned long long	tULong;
				unsigned long		tUInt;
				unsigned short		tUShort;
				unsigned char		tUChar;
			} m_Value;
			size_t m_Size;
		};
		class FloatType
		{
		public:
			enum Type { None, Float };

		private:
			Type m_Type = None;
			float m_Value;

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
			inline void Negate() { m_Value = -m_Value; }
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

		class Scanner : public Lexing::Scanner
		{
		public:
			enum Error {
				error_invalid_hex_char,
				error_invalid_decimal_char,
			};

		private:
			bool m_Float;
			bool m_Hex;
			bool m_Neg;

			long long m_IntVal;
			float m_FloatVal;

			static inline bool IsHexPrefix(char c) {
				return c == 'x' || c == 'X';
			}

		public:
			bool Scan(Lexing::State & state, Scripts::Position & pos) override {
				switch (state) {
				case Lexing::State::before:
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
						state = Lexing::State::inside;
						return true;
					}
					return false;

				case Lexing::State::inside: {
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
					state = Lexing::State::after;
					return true;
				}
				case Lexing::State::after:
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