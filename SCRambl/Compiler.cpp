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
		void Compiler::Init() {
			m_TokenIt = m_Tokens.Begin();
			m_SymbolIt = m_Build->GetSymbolsBegin();
			m_Task(Event::Begin);
			m_State = compiling;

			auto name = m_Build->GetEnvVar("ScriptName").AsString();
			if (name.empty())
				BREAK();
			m_File.open(name + ".scrmbl", std::ios::out | std::ios::binary);
			
			Output<uint64_t>(GetTime());
			Output<uint8_t>(name.size());
			Output(name.c_str(), name.size());
		}
		void Compiler::Reset() {
		}
		void Compiler::Run()
		{
			switch (m_State) {
			case init:
				Init();
				return;
			case compiling:
				if (m_SymbolIt != m_Build->GetSymbolsEnd())
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
			auto symbol = *m_SymbolIt;
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
					Output<uint16_t>(it->second);
				}
				else {
					Output<uint16_t>(AddCommandName(command->GetName()));
				}
				
				Output<uint16_t>(cmd.GetNumArgs());

				for (size_t n = 0; n < cmd.GetNumArgs(); ++n) {
					auto& arg = command->GetArg(n);
					
				}
				break;
			}
			case Tokens::Type::Identifier: {

				break;
			}
			case Tokens::Type::LabelRef: {
				break;
				}
			}

			++m_SymbolIt;
		}
		void Compiler::Finish() {
			for (auto name : m_CommandNameVec) {
				Output<uint16_t>(name.second);
				Output(name.first.c_str(), name.first.size());
				Output<uint8_t>(0);
			}
		}

		Compiler::Compiler(Task& task, Engine& engine, Build* build) :
			m_State(init), m_Task(task), m_Engine(engine), m_Build(build), m_Tokens(build->GetScript().GetTokens())
		{
		}
	}
}