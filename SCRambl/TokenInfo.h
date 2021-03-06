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
	namespace Types { class Type; }

	class IToken
	{
		template<typename>
		friend class TokenBase;

	public:
		IToken() = default;
		virtual ~IToken() { }

	public:
		template<typename T>
		inline T& Get() { return *static_cast<T*>(this); }
		template<typename T>
		inline const T& Get() const { return *static_cast<const T*>(this); }
		template<typename T>
		inline T GetType() const { return static_cast<const TokenBase<T>*>(this)->GetType(); }
	};

	template<typename TTokenType>
	class TokenBase : public IToken
	{
		TTokenType m_Type;

	private:
		using IToken::Get;

	public:
		TokenBase(TTokenType type) : m_Type(type)
		{ }
		virtual ~TokenBase() { }

		inline TTokenType GetType() const { return m_Type; }
	};

	template<typename TTokenType, typename... TValueType>
	class TokenInfo : public TokenBase<TTokenType>
	{
		using IToken::Get;

		std::tuple<TValueType...> m_Value;

	public:
		template<std::size_t N>
		using ValueType = typename std::tuple_element<N, std::tuple<TValueType...>>::type;

		TokenInfo(TTokenType type, TValueType... val) : TokenBase(type), m_Value(val...)
		{ }

		template<std::size_t N>
		inline const ValueType<N>& GetValue() const {
			return std::get<N>(m_Value);
		}
		template<std::size_t N>
		inline ValueType<N>& GetValue() {
			return std::get<N>(m_Value);
		}
	};
	
	enum class TokenType {
		None,
		Eol,
		Whitespace,
		OpenParen,
		CloseParen,
		Separator,
		BlockComment,
		Comment,
		Directive,

		Identifier,
		Number,
		Label,
		Operator, ParseOperator,
		Delimiter,
		String,

		Invalid,
		Max = Invalid
	};
}