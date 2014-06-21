#include "stdafx.h"
#include "Directives.h"

namespace SCRambl
{
	const DirectiveMap g_Directives = {
		{ { "define" },		Directive::DEFINE },
		{ { "if" },			Directive::IF },
		{ { "ifdef" },		Directive::IFDEF },
		{ { "else" },		Directive::ELSE },
		{ { "elif" },		Directive::ELIF },
		{ { "endif" },		Directive::ENDIF },
		{ { "include" },	Directive::INCLUDE },
		{ { "undef" },		Directive::UNDEF }
	};

	const Directive & GetDirective(const std::string & str)
	{
		static Directive invalid = Directive::INVALID;
		auto it = g_Directives.find(str);
		return it != g_Directives.end() ? it->second : invalid;
	}
}