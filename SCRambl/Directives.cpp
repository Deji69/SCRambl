#include "stdafx.h"
#include "Directives.h"

namespace SCRambl
{
	const DirectiveMap g_Directives = {
		{ "define", DIRECTIVE_DEFINE },
		{ "if", DIRECTIVE_IF },
		{ "else", DIRECTIVE_ELSE },
		{ "elif", DIRECTIVE_ELIF },
		{ "endif", DIRECTIVE_ENDIF },
		{ "include", DIRECTIVE_INCLUDE },
	};

	eDirective GetDirective(const std::string & str)
	{
		auto it = g_Directives.find(str);
		return it != g_Directives.end() ? it->second : BAD_DIRECTIVE;
	}
}