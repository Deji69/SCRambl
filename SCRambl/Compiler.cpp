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
			m_XlationIt = m_Build->GetXlationsBegin();
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
				if (m_XlationIt != m_Build->GetXlationsEnd())
					Compile();
				else
					Finish();
				return;
			}
		}
		void Compiler::Compile()
		{
			auto xlate = *m_XlationIt;
			auto translation = xlate.GetTranslation();
			
			for (size_t y = 0; y < translation->GetDataCount(); ++y) {
				auto data = translation->GetData(y);

				bool complete_val = false;
				for (size_t x = 0; x < data->GetNumFields(); ++x) {
					auto field = data->GetField(x);
					auto value = xlate.GetAttribute(field->GetDataSource(), field->GetDataAttribute());
					size_t size = field->HasSizeLimit() ? field->GetSizeLimit() : 0;

					switch (field->GetDataType()) {
					case Types::DataType::Int:
						if (!field->HasSizeLimit()) {
							size = CountBitOccupation(value.AsNumber<size_t>());
							if (size > 32) size = 64;
							else if (size > 16) size = 32;
							else if (size > 8) size = 16;
							else size = 8;
						}

						if (size > 32) {
							uint64_t v = value.AsNumber<uint64_t>();
							Output<uint64_t>(v);
						}
						else if (size > 16) {
							uint32_t v = value.AsNumber<uint32_t>();
							Output<uint32_t>(v);
						}
						else if (size > 8) {
							uint16_t v = value.AsNumber<uint16_t>();
							Output<uint16_t>(v);
						}
						else {
							uint8_t v = value.AsNumber<uint8_t>();
							Output<uint8_t>(v);
						}
						break;
					}
				}
			}
			
			++m_XlationIt;
			return;

			/*Types::Type* type = nullptr;
			switch (symbol->GetType()) {
			case Tokens::Type::Command:
			case Tokens::Type::CommandOverload:
				BREAK();
				break;
			case Tokens::Type::CommandCall: {
				auto cmd = symbol->Extend<Tokens::Command::Call>();
				
				
				auto command = cmd.GetCommand();
				
				auto it = m_CommandNames.find(command->Name());
				if (it != m_CommandNames.end())
					Output<uint16_t>(it->second);
				else
					Output<uint16_t>(AddCommandName(command->Name()));
				
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
			default:
				break;
			}

			++m_SymbolIt;*/
		}
		void Compiler::Finish() {
			Output<uint32_t>(m_CommandNameVec.size());
			for (auto name : m_CommandNameVec) {
				Output<uint16_t>(name.second);
				Output(name.first.c_str(), name.first.size());
				Output<uint8_t>(0);
			}

			m_Task(Event::Finish);
			m_State = finished;
			m_File.close();
		}

		Compiler::Compiler(Task& task, Engine& engine, Build* build) :
			m_State(init), m_Task(task), m_Engine(engine), m_Build(build), m_Tokens(build->GetScript().GetTokens())
		{
		}
	}
}