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

	//template<Token::Type TokType>
	class TokenInfo
	{
	public:
		enum Type {
			Identifier, Label,
		};

		class Data
		{

		};

	private:
		Type					m_Type;
		Script::Range			m_Code;				// source code range in Script

	public:
		inline const Script::Range &	GetRange() const			{ return m_Code; }
		inline std::string				GetSource() const			{ return m_Code.Format(); }
		inline Type						GetType() const				{ return m_Type; }
	};

	class Tokens
	{
		std::vector<TokenInfo>			m_Tokens;
	};
}