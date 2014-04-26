/****************************************************/
// SCRambl - Directives.h
// Definitions for SCR directives
/****************************************************/
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