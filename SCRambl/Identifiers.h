/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
// Definitions for SCR identifiers
#pragma once
#include <string>
#include <vector>
#include <map>

#include "Parser.h"
#include "Lexer.h"

namespace SCRambl
{
	typedef std::string Identifier;

	class IdentifierComp : std::binary_function<Identifier, Identifier, bool>
	{
		class identifier_pred : public std::binary_function<unsigned char, unsigned char, bool>
		{
		public:
			bool operator() (const unsigned char& a, const unsigned char& b) const
			{
				return tolower(a) < tolower(b);
			}
		};
	public:
		bool operator() (const Identifier & a, const Identifier & b) const
		{
			return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),	identifier_pred());
		}
	};

	typedef std::vector<std::string> IdentifierVec;

	template<class T>
	using IdentifierMap = std::map<Identifier, T, IdentifierComp>;

	class IdentifierScanner : public Lexer::Scanner
	{
	public:
		bool Scan(const Lexer::LexerState & state, const std::string & str, std::string::const_iterator & it) override
		{
			switch (state)
			{
				// return true if IsIdentifierStart
			case Lexer::LexerState::before:
				if (IsIdentifierStart(*it))
				{
					++it;
					return true;
				}
				break;
				// return true once we've read all the identifier characters
			case Lexer::LexerState::inside:
				return !IsIdentifier(*it++);
				// make sure that a separator followed the identifier chars
			case Lexer::LexerState::after:
				if (!IsSeparator(*it)) return false;
				return true;
			}
			return false;
		}
	};
}