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
	enum class Token
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

	class TokenBase
	{
		Token		m_Type;

	public:
		TokenBase(Token id) : m_Type(id)
		{ }
	};

	template<Token TokType>
	class TokenInfo
	{
	public:
		class Data
		{

		};

	private:
		//TokType				m_Type;
		//Script::Position	m_Inside;
		//Script::Position	m_After;

	public:
		inline Token		GetType() const		{ return TokType; }
	};

	class Tokens
	{
	};
}