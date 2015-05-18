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
			Output<uint64_t>(GetTime());
			std::string test = "TEST";
			Output(test.c_str(), test.length());
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
					Finish();
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
			Output<uint8_t>(symbol->GetType());

			switch (symbol->GetType()) {
			case Tokens::Type::Command:
			case Tokens::Type::CommandOverload:
				BREAK();
				break;
			case Tokens::Type::CommandCall: {
				auto cmd = symbol->Extend<Tokens::Command::Call<Command>>();
				auto command = cmd.GetCommand();

				auto it = m_CommandNames.find(command->GetName());
				if (it != m_CommandNames.end()) {
					Output<uint32_t>(it->second);
				}
				else {
					Output<uint32_t>(m_CommandNames.size());
					m_CommandNames.emplace(command->GetName(), m_CommandNames.size());
				}
				
				Output<uint32_t>(cmd.GetNumArgs());
				break;
				}
			case Tokens::Type::LabelRef: {
				break;
				}
			}

			++m_TokenIt;
		}
		void Compiler::Finish() {
			for (auto name : m_CommandNames) {
				Output(name.first.c_str(), name.first.size());
				Output<uint8_t>(0);
			}
		}

		Compiler::Compiler(Task & task, Engine & engine, Script & script) :
			m_State(init), m_Task(task), m_Engine(engine), m_Script(script), m_Tokens(script.GetTokens())
		{
		}
	}
}