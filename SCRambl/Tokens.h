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
	namespace Token
	{
		enum Type
		{
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

		template<typename T = Type>
		class Data
		{
			T					m_Type;

		public:
			Data(T type) : m_Type(type)
			{ }
		};
	}

	class TokenBase
	{
		Token::Type		m_Type;

	public:
		TokenBase(Token::Type id) : m_Type(id)
		{ }
	};

	template<Token::Type TokType>
	class TokenInfo
	{
	public:
		class Data
		{

		};

	private:
		//TokType				m_Type;
		//Script::Position		m_Inside;
		//Script::Position		m_After;

	public:
		inline Token::Type		GetType() const		{ return TokType; }
	};

	class Tokens
	{
	};
}