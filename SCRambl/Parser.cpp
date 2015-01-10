#include "stdafx.h"
#include "Standard.h"
#include "Parser.h"
#include "Exception.h"
#include "Scripts.h"
#include "Preprocessor.h"
#include "TokensB.h"

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
				auto& token = m_CommandTokenIt->get()->Get<Tokens::Identifier::Info<>>();
				auto range = token.GetValue<0>();
				*m_CommandTokenIt = Script::Tokens::MakeShared < CommandInfo >(range.Begin(), ParsedType::Command, range, m_CurrentCommand);
				
				m_State = parsing;
				return;
			}

			m_CurrentCommand = *m_OverloadCommandsIt;
			Parse();
		}
		void Parser::Parse()
		{
			auto& types = m_Engine.GetTypes();
			auto ptr = *m_TokenIt;
			auto type = ptr->GetType<Tokens::Type>();
			bool newline = false;

			if (IsCommandParsing()) {
				m_CommandArgIt->IsReturn();
			}

			Types::ValueSet val_type;

			switch (type) {
			case Tokens::Type::Label: {
				auto& token = ptr->Get<Tokens::Label::Info>();
				auto label = token.GetValue<Tokens::Label::Parameter::LabelValue>();
				//auto b = token.GetValue<2>();
				
				if (!m_OnNewLine) {
					SendError(Error::label_on_line, label);
				}

				val_type = Types::ValueSet::Label;
				break;
			}
			case Tokens::Type::Identifier: {
				Commands::Vector vec;
				auto& token = ptr->Get<Tokens::Identifier::Info<>>();
				auto range = token.GetValue<0>();
				auto name = range.Format();

				if (auto ptr = m_Labels.Find(name)) {
					// this is a label pointer!
					m_Jumps.emplace_back(ptr, m_TokenIt);
				}
				else if (m_Commands.FindCommands(name, vec) > 0) {
					if (vec.size() == 1) *m_TokenIt = Script::Tokens::MakeShared < CommandInfo >(range.Begin(), ParsedType::Command, range, vec[0]);
					else *m_TokenIt = Script::Tokens::MakeShared < OLCommandInfo >(range.Begin(), ParsedType::OLCommand, range, vec);

					if (ParseCommandOverloads(vec)) BeginCommandParsing();
				}
				else if (false /*check variables */) {

				}
				else if (IsCommandParsing()/* && m_CommandArgIt->GetType().IsCompatible()*/) {

				}
				else {
					SendError(Error::invalid_identifier, range);
				}

				val_type = Types::ValueSet::Label;
				
				break;
			}
			case Tokens::Type::String: {
				auto& token = ptr->Get<Tokens::String::Info>();
				auto range = token.GetValue<0>();
				auto string = range.Format();

				

				val_type = Types::ValueSet::Text;
				break;
			}
			case Tokens::Type::Number: {
				auto type = Tokens::Number::GetValueType(*ptr);
				bool is_int = type != Numbers::Float;
				Numbers::IntegerType int_value;
				Numbers::FloatType float_value;
				
				size_t size;
				if (is_int) {
					auto info = ptr->Get<Tokens::Number::Info<Numbers::IntegerType>>();
					int_value = info.GetValue < Tokens::Number::Parameter::NumberValue >();
					size = CountBitOccupation(int_value.GetValue<Numbers::IntegerType::MaxType>());
				}
				else {
					auto info = ptr->Get<Tokens::Number::Info<Numbers::FloatType>>();
					float_value = info.GetValue < Tokens::Number::Parameter::NumberValue >();
					size = 32;
				}

				if (IsCommandParsing()) {
					auto& type = m_CommandArgIt->GetType();
					bool b = type.IsBasicType();
					auto name = m_CurrentCommand->GetName();
					name.size();
				}
				
				//Types::NumberValue number_value((is_int ? Types::NumberValue::Integer : Types::NumberValue::Float), CountBitOccupation(int_value));
				Types::Value::Shared best_value_match;
				size_t smallest_fitting_value_size = 0;

				std::vector<Types::Value::Shared> vec;
				size_t n;
				if (IsCommandParsing()) {
					auto& type = m_CommandArgIt->GetType();
					n = types.GetValues(Types::ValueSet::Number, size, vec, [&best_value_match, &smallest_fitting_value_size, is_int, &vec](Types::Value::Shared value){
						// Keep this value?
						auto& num_value = value->Extend<Types::NumberValue>();
						if (num_value.GetNumberType() != (is_int ? Types::NumberValueType::Integer : Types::NumberValueType::Float))
							return false;

						// Oh, is this the only one we need?
						auto &ntype = value->GetType();
						if (!best_value_match || smallest_fitting_value_size > value->GetSize()) {
							best_value_match = value;
							smallest_fitting_value_size = value->GetSize();
						}
						return true;
					});

					auto& number_value = best_value_match->Extend<Types::NumberValue>();
					auto val_token = is_int ? number_value.CreateToken<Tokens::Number::Value<Types::NumberValue>>(ptr->Get<Tokens::Number::Info<Numbers::IntegerType>>())
						: number_value.CreateToken<Tokens::Number::Value<Types::NumberValue>>(ptr->Get<Tokens::Number::Info<Numbers::FloatType>>());


					ptr->Get<Tokens::Number::Info<Numbers::FloatType>>().GetValue<Tokens::Number::ScriptRange>();
					auto info = ptr->Get<Tokens::Number::Info<Numbers::IntegerType>>();
					info.GetType();

					if (best_value_match) {
						//best_value_match->GetType
					}
				}
				else
				{
					n = types.GetValues(Types::ValueSet::Number, size, vec, [this, is_int, &vec](Types::Value::Shared value){
						// Keep this value?
						auto& num_value = value->Extend<Types::NumberValue>();
						if (num_value.GetNumberType() != (is_int ? Types::NumberValueType::Integer : Types::NumberValueType::Float))
							return false;
						return true;
					});
				}

				if (!n) {
					SendError(Error::unsupported_value_type);
				}
				
				val_type = Types::ValueSet::Number;

				if (IsCommandParsing()) {
					
				}
				break;
			}
			case Tokens::Type::Character: {
				auto& token = ptr->Get<Tokens::Character::Info<Character>>();
				switch (auto ch = token.GetValue<Tokens::Character::Parameter::CharacterValue>()) {
				case Character::EOL:
					newline = true;
					break;
				default:
					SendError(Error::invalid_character, ch);
					break;
				}
				break;
			}
			case Tokens::Type::Operator: {
				auto& token = ptr->Get<Tokens::Operator::Info<Operator::Type>>();
				auto type = token.GetValue<Tokens::Operator::Parameter::OperatorType>();
				auto rg = token.GetValue<Tokens::Operator::Parameter::ScriptRange>();
				rg.Begin();
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