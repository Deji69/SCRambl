#include "stdafx.h"
#include "Parser.h"
#include "Exception.h"

#include <cctype>

namespace SCRambl
{
	void Parser::Run()
	{

	}
	void Parser::Reset()
	{

	}

	Parser::Parser(Engine & engine, Script & script):
		m_Engine(engine), m_Script(script)
	{
	}
}