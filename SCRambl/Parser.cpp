#include "stdafx.h"
#include "Standard.h"
#include "Parser.h"
#include "Exception.h"
#include "Scripts.h"
#include "Preprocessor.h"

#include <cctype>

namespace SCRambl
{
	namespace Parser
	{
		void Parser::Init()
		{
			m_TokenIt = m_Tokens.Begin();

			m_Task(Event::Begin);
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
			auto ptr = *m_TokenIt;
			auto type = ptr->GetType<Token::Type>();

			switch (type) {
			case Token::Label:
			{
				auto& label = ptr->Get<Token::LabelInfo>();
				auto& name = label.GetValue<0>().Format();
				
				break;
			}
			default:
				
				break;
			}

			++m_TokenIt;
		}

		Parser::Parser(Task & task, Engine & engine, Script & script) :
			m_Task(task), m_Engine(engine), m_Script(script), m_State(init),
			m_Tokens(script.GetTokens())
		{
		}

		// Printf-styled error reporting
		void Parser::SendError(Error type)
		{
			// send
			std::vector<std::string> params;
			m_Task(Event::Error, Basic::Error(type), params);
		}
		template<typename First, typename... Args>
		void Parser::SendError(Error type, First&& first, Args&&... args)
		{
			// storage for error parameters
			std::vector<std::string> params;
			// format the error parameters to the vector
			m_Engine.Format(params, first, args...);
			// send
			m_Task(Event::Error, Basic::Error(type), params);
		}
	}
}