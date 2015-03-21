#include "stdafx.h"
#include "Standard.h"
#include "Compiler.h"
#include "Exception.h"
#include "Scripts.h"
#include "TokensB.h"

#include <cctype>

namespace SCRambl
{
	namespace Compiler
	{
		void Compiler::Init()
		{
			m_TokenIt = m_Tokens.Begin();
			m_Task(Event::Begin);
			m_State = compiling;

			m_File.open("script.scrmbl", std::ios::out | std::ios::binary);
			Output(GetTime());
			std::string test = "TEST";
			Output(test, test.length());

			m_Script.OutputFile();
		}
		void Compiler::Reset()
		{
		}
		void Compiler::Run()
		{
			switch (m_State) {
			case init:
				Init();
				return;
			case compiling:
				if (m_TokenIt != m_Tokens.End())
					Compile();
				else
				{
					m_Task(Event::Finish);
					m_State = finished;
					m_File.close();
				}
				return;
			}
		}
		void Compiler::Compile()
		{
			while (!m_TokenIt.Get()->GetSymbol()) {
				if (++m_TokenIt == m_Tokens.End())
					return;
			}
			
			auto symbol = m_TokenIt.Get()->GetSymbol();
			Output(symbol->GetType());

			switch (symbol->GetType()) {
			case Tokens::Type::Command:
			case Tokens::Type::CommandOverload:
				BREAK();
				break;
			case Tokens::Type::CommandCall: {
				auto cmd = symbol->Extend<Tokens::Command::Call<Command>>();
				cmd.GetCommand()->GetNumArgs();
				Output(cmd);
				break;
				}
			}

			++m_TokenIt;
		}

		Compiler::Compiler(Task & task, Engine & engine, Script & script) :
			m_State(init), m_Task(task), m_Engine(engine), m_Script(script), m_Tokens(script.GetTokens())
		{
		}
	}
}