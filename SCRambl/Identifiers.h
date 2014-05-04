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
}