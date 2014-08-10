#include "stdafx.h"
#include "Parser.h"
#include "Exception.h"

#include <cctype>

namespace SCRambl
{
	void Parser::Init()
	{
		m_TokenIt = m_Tokens.Begin();
		m_State = parsing;
	}
	void Parser::Run()
	{
		switch (m_State) {
		case init:
			Init();
			return;
		case parsing:
			Parse();
			return;
		}
	}
	void Parser::Reset()
	{

	}
	void Parser::Parse()
	{
		
	}

	Parser::Parser(Engine & engine, Script & script):
		m_Engine(engine), m_Script(script),
		m_Tokens(script.GetTokens())
	{
	}
}