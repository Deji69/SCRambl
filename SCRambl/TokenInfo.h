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
	enum class PreprocessingToken {
		None, Directive, Identifier, Label, Number, Operator, String
	};

	class IToken
	{
	public:
		IToken() = default;
		virtual ~IToken() { }
	};

	template<typename TTokenType>
	class TokenBase : public IToken
	{
		TTokenType			m_Type;

	public:
		TokenBase(TTokenType type) : m_Type(type)
		{ }
		virtual ~TokenBase() { }

		inline TTokenType				GetType() const				{ return m_Type; }
	};

	template<typename TTokenType, typename... TValueType>
	class TokenInfo : public TokenBase<TTokenType>
	{
		//TValueType					m_Value;
		std::tuple<TValueType...>	m_Value;

	public:
		TokenInfo(TTokenType type, TValueType&&... val) : TokenBase(type), m_Value(val...)
		{ }

		inline std::tuple<TValueType...> &			GetValue()					{ return m_Value; }
		inline const std::tuple <TValueType...> &	GetValue() const			{ return m_Value; }
	};
	
	enum class TokenType {
		None,
		Whitespace,
		Eol,
		Identifier,
		Label,
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
}