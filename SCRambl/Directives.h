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
	enum eDirective : int
	{
		BAD_DIRECTIVE,
		DIRECTIVE_DEFINE,
		DIRECTIVE_IF,
		DIRECTIVE_ELSE,
		DIRECTIVE_ELIF,
		DIRECTIVE_ENDIF,
		DIRECTIVE_IFDEF,
		DIRECTIVE_INCLUDE,
	};
	
	typedef IdentifierMap<eDirective> DirectiveMap;

	eDirective GetDirective(const std::string &);
}