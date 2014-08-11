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
	class IToken
	{
		template<typename>
		friend class TokenBase;

	public:
		using Shared = std::shared_ptr < IToken > ;

	protected:
		IToken() = default;
		virtual ~IToken() { }

	public:
		template<typename T>
		inline T & Get()								{ return *dynamic_cast<T*>(this); }
		template<typename T>
		inline const T & Get() const					{ return *dynamic_cast<T*>(this); }
		template<typename T>
		inline T GetType() const						{ return static_cast<const TokenBase<T>*>(this)->GetType(); }
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
	private:
		std::tuple<TValueType...>	m_Value;

	public:
		TokenInfo(TTokenType type, TValueType&&... val) : TokenBase(type), m_Value(val...)
		{ }

		template<std::size_t N>
		inline typename std::tuple_element<N, std::tuple<TValueType...>>::type & GetValue() 				{ return std::get<N>(m_Value); }
		template<std::size_t N>
		inline typename const std::tuple_element<N, std::tuple<TValueType...>>::type & GetValue() const		{ return std::get<N>(m_Value); }
	};
	
	enum class TokenType {
		None,
		Eol,
		Whitespace,
		OpenParen,
		CloseParen,
		BlockComment,
		Comment,
		Directive,

		Identifier,
		Number,
		Label,
		Operator,
		String,

		Invalid,
		Max = Invalid
	};
}