/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include <map>
#include "Identifiers.h"

namespace SCRambl
{
	class Directive
	{
	public:
		enum ID : int
		{
			INVALID,
			DEFINE,
			IF,
			IFDEF,
			ELSE,
			ELIF,
			ENDIF,
			INCLUDE,
			UNDEF,
		}	m_ID;

		Directive(ID);
		operator ID() const;
	};
	
	typedef Identifier::Map<Directive> DirectiveMap;

	const Directive & GetDirective(const std::string &);
}