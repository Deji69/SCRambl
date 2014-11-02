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

			m_OnNewLine = true;
		}
		void Parser::Run()
		{
			switch (m_State) {
			case init:
				Init();
				return;
			case parsing:
				if (m_TokenIt != m_Tokens.End())
					Parse();
				else
				{
					m_Task(Event::Finish);
					m_State = finished;
				}
				return;
			case overloading:
				ParseOverloadedCommand();
				return;
			}
		}
		void Parser::Reset()
		{

		}
		void Parser::ParseOverloadedCommand()
		{
			if (m_OverloadCommands.size() <= 1) {
				m_CurrentCommand = m_OverloadCommands[0];
				m_OverloadCommands.clear();

				// replace overload token with single command token
				auto& token = m_CommandTokenIt->get()->Get<Token::Identifier::Info<>>();
				auto range = token.GetValue<0>();
				*m_CommandTokenIt = Script::Tokens::MakeShared < Token::CommandInfo >(range.Begin(), Token::Command, range, m_CurrentCommand);
				
				m_State = parsing;
				return;
			}

			m_CurrentCommand = *m_OverloadCommandsIt;
			Parse();
		}
		void Parser::Parse()
		{
			auto ptr = *m_TokenIt;
			auto type = ptr->GetType<Token::Type>();
			bool newline = false;

			if (IsCommandParsing()) {
				m_CommandArgIt->IsReturn();
			}

			switch (type) {
			case Token::Type::Label: {
				auto& token = ptr->Get<Token::Label::Info>();
				auto label = token.GetValue<Token::Label::Value::LabelValue>();
				//auto b = token.GetValue<2>();
				
				if (!m_OnNewLine) {
					SendError(Error::label_on_line, label);
				}
				break;
			}
			case Token::Type::Identifier: {
				Commands::Vector vec;
				auto& token = ptr->Get<Token::Identifier::Info<>>();
				auto range = token.GetValue<0>();
				auto name = range.Format();

				if (auto ptr = m_Labels.Find(name))
				{
					// this is a label pointer!
					m_Jumps.emplace_back(ptr, m_TokenIt);
				}
				else if (m_Commands.FindCommands(name, vec) > 0)
				{
					if (vec.size() == 1) *m_TokenIt = Script::Tokens::MakeShared < Token::CommandInfo >(range.Begin(), Token::Command, range, vec[0]);
					else *m_TokenIt = Script::Tokens::MakeShared < Token::OLCommandInfo >(range.Begin(), Token::OLCommand, range, vec);

					if (ParseCommandOverloads(vec)) BeginCommandParsing();
				}
				else
				{
					SendError(Error::invalid_identifier, range);
				}
				break;
			}
			case Token::Type::Number: {
				auto type = Token::Number::GetValueType(*ptr);
				bool is_int = type != Numbers::Float;
				Numbers::IntegerType int_value;
				Numbers::FloatType float_value;
				
				if (is_int)
				{
					auto info = ptr->Get<Token::Number::Info<Numbers::IntegerType>>();
					int_value = info.GetValue < Token::Number::Value::NumberValue >();
				}
				else
				{
					auto info = ptr->Get<Token::Number::Info<Numbers::FloatType>>();
					float_value = info.GetValue < Token::Number::Value::NumberValue >();
				}

				if (IsCommandParsing()) {
					auto name = m_CurrentCommand->GetName();
					name.size();
				}
				//bool is_int = token.GetValue<
				break;
			}
			case Token::Type::Character: {
				auto& token = ptr->Get<Token::Character::Info<Character>>();
				switch (auto ch = token.GetValue<Token::Character::Value::CharacterValue>()) {
				case Character::EOL:
					newline = true;
					break;
				default:
					SendError(Error::invalid_character, ch);
					break;
				}
				break;
			}
			default:
				
				break;
			}

			m_OnNewLine = newline;
			++m_TokenIt;
		}

		Parser::Parser(Task & task, Engine & engine, Script & script) :
			m_Task(task), m_Engine(engine), m_Script(script), m_State(init),
			m_Tokens(script.GetTokens()),
			m_Labels(script.GetLabels()),
			m_Commands(m_Engine.GetCommands())
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