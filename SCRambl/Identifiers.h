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
#include <hash_map>

//#include "Parser.h"
//#include "Lexer.h"
#include "Symbols.h"

namespace SCRambl
{
	//typedef std::string Identifier;

	class Identifier
	{
		static std::hash<std::string>	g_Hasher;

		std::string						m_Name;
		size_t							m_Hash;

		class Hasher
		{
		public:
			size_t operator()(const Identifier& k) const		{ return g_Hasher(k.Name()); }
		};

	public:
		template<class T>
		using Map = std::unordered_map<Identifier, T, Hasher>;

		Identifier(std::string name) : m_Name(name), m_Hash(g_Hasher(name))
		{
		}

		inline const std::string& Name() const		{ return m_Name; }
		inline size_t Hash() const					{ return m_Hash; }
		inline bool operator==(const Identifier & rhs) const {
			return (Hash() == rhs.Hash() && Name() == rhs.Name());
		}
	};

	/*typedef std::vector<std::string> IdentifierVec;*/

	//template<class T>
	//using Identifier = std::unordered_map<
	//using IdentifierMap = std::map<Identifier, T, IdentifierComp>;

	/*class IdentifierScanner : public Lexer::Scanner
	{
	public:
		bool Scan(const Lexer::State & state, Symbol & str, std::string::const_iterator & it) override
		{
			switch (state)
			{
				// return true if IsIdentifierStart
			case Lexer::State::before:
				if (IsIdentifierStart(*it))
				{
					++it;
					return true;
				}
				break;
				// return true once we've read all the identifier characters
			case Lexer::State::inside:
				return !IsIdentifier(*it++);
				// make sure that a separator followed the identifier chars
			case Lexer::State::after:
				if (!IsSeparator(*it)) return false;
				return true;
			}
			return false;
		}
	};*/
}