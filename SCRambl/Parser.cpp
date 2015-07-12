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
		Parser::Parser(Task& task, Engine& engine, Build& build) :
			m_Task(task), m_Engine(engine), m_Build(build), m_State(init),
			m_Tokens(build.GetScript().GetTokens()),
			//m_Labels(build.GetScript().GetLabels()),
			m_Commands(build.GetCommands()),
			m_Types(build.GetTypes())
		{ }
		void Parser::Init() {
			m_TokenIt = m_Tokens.Begin();

			m_Task(Event::Begin);
			m_State = parsing;

			m_OnNewLine = true;
		}
		void Parser::Finish() {
			m_Build.ParseCommands(m_CommandTokenMap);
		}
		void Parser::Run() {
			switch (m_State) {
			case init:
				Init();
				return;
			case parsing:
				if (m_TokenIt != m_Tokens.End())
					Parse();
				else {
					Finish();
					m_Task(Event::Finish);
					m_State = finished;
				}
				return;
			case overloading:
				ParseOverloadedCommand();
				return;
			}
		}
		void Parser::ParseOverloadedCommand() {
			if (m_OverloadCommands.size() <= 1) {
				m_CurrentCommand = m_OverloadCommands[0];
				m_OverloadCommands.clear();

				// replace overload token with single command token
				auto& token = m_CommandTokenIt->GetToken()->Get<Tokens::Identifier::Info<>>();
				auto range = token.GetValue<Tokens::Identifier::ScriptRange>();
				// TODO: fix this weirdness
				m_CommandTokenIt->SetToken(m_Build.CreateToken<CommandInfo>(range.Begin(), Tokens::Type::Command, range, m_CurrentCommand).GetToken());
				
				m_State = parsing;
				return;
			}

			m_CurrentCommand = *m_OverloadCommandsIt;
			Parse();

			if (AreCommandArgsParsed()) {
				if (!m_OnNewLine) FailCommandOverload();
				if (GetNumberOfOverloadFailures() > 0)
					MarkOverloadIncompatible();
				else
					NextCommandOverload();
			}
		}
		bool Parser::ParseCommandOverloads(const Commands::Vector& vec) {
			if (vec.size() == 1) {
				m_CurrentCommand = vec[0];
			}
			else {
				if (!m_OverloadCommands.empty()) {
					m_OverloadCommands = vec;
					m_OverloadCommandsIt = m_OverloadCommands.begin();
					m_CurrentCommand = *m_OverloadCommandsIt;
					m_State = overloading;
				}
				else {
					m_CurrentCommand = nullptr;
					return false;
				}
			}
			return true;
		}
		void Parser::BeginCommandParsing() {
			m_CommandTokenIt = m_TokenIt;
			m_NumCommandArgs = 0;
			m_NumOverloadFailures = 0;
			m_EndOfCommandArgs = false;

			if (m_State == overloading) {
				m_CurrentCommand = *m_OverloadCommandsIt;
			}
			if (m_CurrentCommand->NumArgs()) {
				m_CommandArgIt = m_CurrentCommand->BeginArg();
				m_ParsingCommandArgs = true;
			}
		}

		States Parser::Parse_Neutral_CheckCharacter(IToken* tok) {
			if (IsCharacterEOL(tok)) {
				switch (m_ActiveState) {
				case state_parsing_operator:
					if (m_OperationParseState.RequireRVal())
						BREAK();
					else if (m_OperationParseState.CheckForRVal())
						CreateSymbol<Operation>(m_OperationParseState.lh_var, m_OperationParseState.operation);
					break;
				case state_parsing_command:
				case state_parsing_command_args:
					m_CommandTokenMap.emplace(m_CommandParseState.command->Name(), m_CommandTokenIt);
					//CreateSymbol<Tokens::Command::Call>(m_CommandTokenIt->GetToken(), m_CommandParseState.parameters.size());

					for (auto& param : m_CommandParseState.parameters) {
						//CreateSymbol()
					}
					break;
				}
				m_ActiveState = state_neutral;
			}
			return state_neutral;
		}
		States Parser::Parse_Neutral_CheckIdentifier(IToken* tok) {
			Commands::Vector vec;
			auto& token = tok->Get<Tokens::Identifier::Info<>>();
			auto range = token.GetValue<Tokens::Identifier::ScriptRange>();
			auto name = range.Format();

			if (auto type = GetType(name)) {
				m_TypeParseState = TypeParseState(type, tok);
				return state_parsing_type;
			}

			else if (auto ptr = m_Build.GetScriptLabel(name)) {
				// this is a label pointer!
				m_Jumps.emplace_back(ptr->Ptr(), m_TokenIt);

				auto tok = CreateToken<Tokens::Label::Info>(Tokens::Type::LabelRef, range, ptr->Ptr());
				m_TokenIt.Get()->SetToken(tok);
				auto sym = CreateSymbol<Tokens::Label::Jump>(tok, 0);
				m_TokenIt.Get()->SetSymbol(sym);

				AddLabelRef(ptr, m_TokenIt);
				return state_parsing_label;
			}
			else if (m_Commands.FindCommands(name, vec) > 0) {
				// make a token and store it
				if (vec.size() == 1)
					m_TokenIt.Get()->SetToken(CreateToken<Tokens::Command::Info>(Tokens::Type::Command, range, vec[0]));
				else
					m_TokenIt.Get()->SetToken(CreateToken<Tokens::Command::OverloadInfo>(Tokens::Type::CommandOverload, range, vec));

				if (ParseCommandOverloads(vec)) {
					BeginCommandParsing();
					m_CommandTokens.emplace_back(m_TokenIt);
				}
				else BREAK();
				return state_parsing_command;
			}
			else if (m_Variable = m_Build.GetScriptVariable(name)) {
				m_VariableTokenIt = m_TokenIt;
				return state_parsing_variable;
			}
			else if (IsCommandParsing()/* && m_CommandArgIt->GetType().IsCompatible()*/) {

			}
			else {
				SendError(Error::invalid_identifier, range);
			}
			return state_neutral;
		}
		States Parser::Parse_Neutral_CheckDelimiter(IToken* tok) {
			if (m_ActiveState == state_parsing_variable) {
				if (IsSubscriptDelimiter(tok))
					return state_parsing_subscript;
				else
					BREAK();
			}
			else {
				if (IsScopeDelimiterClosing(tok)) {
					m_Build.OpenVarScope();
				}
				else if (IsScopeDelimiter(tok)) {
					m_Build.CloseVarScope();
				}
				else BREAK();
			}
			return state_neutral;
		}
		States Parser::Parse_Neutral_CheckOperator(IToken* tok) {
			if (m_CurrentOperator = GetOperator(tok)) {
				m_OperatorTokenIt = m_TokenIt;
				return state_parsing_operator;
			}
			else SendError(Error::invalid_operator, GetOperatorRange(tok));
			return state_neutral;
		}
		States Parser::Parse_Neutral_CheckNumber(IToken* tok) {
			if (auto info = GetIntInfo(tok)) {
				m_NumberParseState.Start(info);
				return state_parsing_number;
			}
			else if (auto info = GetFloatInfo(tok)) {
				m_NumberParseState.Start(info);
				return state_parsing_number;
			}
			return state_neutral;
		}
		States Parser::Parse_Neutral_CheckString(IToken* tok) {
			if (m_ActiveState != state_parsing_command_args && m_ActiveState != state_parsing_operator) {
				BREAK();
			}
			return state_parsing_string;
		}
		States Parser::Parse_Neutral() {
			States new_state = state_neutral;
			switch (m_TokenIt->GetToken()->GetType<Tokens::Type>()) {
			case Tokens::Type::Character:
				new_state = Parse_Neutral_CheckCharacter(m_TokenIt->GetToken());
				break;
			case Tokens::Type::Identifier:
				new_state = Parse_Neutral_CheckIdentifier(m_TokenIt->GetToken());
				break;
			case Tokens::Type::Delimiter:
				new_state = Parse_Neutral_CheckDelimiter(m_TokenIt->GetToken());
				break;
			case Tokens::Type::Operator:
				new_state = Parse_Neutral_CheckOperator(m_TokenIt->GetToken());
				break;
			case Tokens::Type::Number:
				new_state = Parse_Neutral_CheckNumber(m_TokenIt->GetToken());
				break;
			case Tokens::Type::String:
				new_state = Parse_Neutral_CheckString(m_TokenIt->GetToken());
				break;
			}
			if (new_state == state_neutral)
				++m_TokenIt;
			return new_state;
		}
		States Parser::Parse_Type() {
			++m_TokenIt;
			return state_parsing_type_varlist;
		}
		States Parser::Parse_Number() {
			Types::Type* type = nullptr;
			size_t size;
			Operand operand;
			if (!m_NumberParseState.IsFloat()) {
				auto info = m_NumberParseState.IntInfo;
				auto val = info->GetValue<Tokens::Number::NumberValue>();
				operand = info;
				size = val.Size();
			}
			else {
				auto info = m_NumberParseState.FloatInfo;
				auto val = info->GetValue<Tokens::Number::NumberValue>();
				operand = info;
				size = val.Size();
			}

			auto value = GetBestValue(Types::ValueSet::Number, size);
			type = value->GetType();
				
			if (m_ActiveState == state_parsing_operator) {
				if (m_OperationParseState.IsInChain()) {
					// var = N + N
					m_OperationParseState.Chain(m_CurrentOperator, {operand, value});
					++m_TokenIt;
				}
				else {
					// var = N
					if (m_OperationParseState.RequireRVal() || m_OperationParseState.CheckForRVal()) {
						if (auto op = m_CurrentOperator->GetOperation(m_OperationParseState.lh_var->Ptr(), type))
							m_OperationParseState.FinishRHS(op, type);
					}
					if (PeekToken(Tokens::Type::Operator)) {
						m_OperationParseState.PrepareChain();
						m_ActiveState = state_parsing_operator;
						++m_TokenIt;
					}
					else {
						m_Build.CreateSymbol<Operation>(m_OperationParseState.operation, m_OperationParseState.lh_var, operand);
						m_ActiveState = state_neutral;
						++m_TokenIt;
					}
				}
				return state_neutral;
			}
			return state_parsing_number;
		}
		States Parser::Parse_String() {
			if (m_ActiveState == state_parsing_command_args) {
				auto str = GetTextString(m_TokenIt->GetToken());
				auto value = GetBestValue(Types::ValueSet::Text, str.size());
				if (!value) BREAK();
				if (!m_CommandParseState.AddParameter(str, value)) BREAK();
				++m_TokenIt;
				return state_neutral;
			}
			else if (m_ActiveState == state_parsing_operator) {

			}
			else BREAK();
			return state_parsing_string;
		}
		States Parser::Parse_Label() {
			return state_parsing_label;
		}
		States Parser::Parse_Variable() {
			if (m_ActiveState == state_parsing_operator) {
				if (m_OperationParseState.IsInChain()) {
					m_OperationParseState.Chain(m_CurrentOperator, { m_Variable, m_Variable->Get().Value() });
				}
				else {
					m_ActiveState = state_neutral;
					if (m_OperationParseState.looksPrefixed) {
						// rhs of a =var unary operation
						if (auto op = m_CurrentOperator->GetUnaryOperation(m_Variable->Ptr(), true)) {
							m_Build.CreateSymbol<Operation>(op, m_Variable);
							return state_neutral;
						}
						else BREAK();		// error?
					}
					else {
						// rhs of a var=var operation
					}
				}
				return state_neutral;
			}
			else if (m_ActiveState == state_neutral) {
				// lhs of a var= operation?
				++m_TokenIt;
				m_ActiveState = state_parsing_variable;
				m_OperationParseState.StartWithVariable();
				return state_neutral;
			}
			return state_parsing_variable;
		}
		States Parser::Parse_Type_Varlist() {
			// end of var declarations?
			if (IsEOLReached()) {
				return state_neutral;
			}
			else if (IsCharacter(m_TokenIt->GetToken())) {
				SendError(Error::invalid_character, GetCharacterValue(m_TokenIt->GetToken()));
				return state_neutral;
			}

			// expect an identifier for a var name
			auto type = GetCurrentTokenType();
			if (type != Tokens::Type::Identifier) {
				SendError(Error::expected_identifier);
				BREAK();
			}
			else {
				// collect var declaration info
				auto name = GetIdentifierName(m_TokenIt->GetToken());
				size_t array_size = 0;
				GetDelimitedArrayIntegerConstant(array_size);

				auto var = m_Build.AddScriptVariable(name, m_TypeParseState.type, array_size);
				if (!var) {
					//SendError();
					BREAK();
				}
			}
			++m_TokenIt;
			return state_parsing_type_varlist;
		}
		States Parser::Parse_Command() {
			m_ActiveState = state_parsing_command_args;
			m_CommandParseState.Begin(m_CurrentCommand, m_CurrentCommand->BeginArg());
			auto attributes = m_CurrentCommand->GetAttributes();
			CommandValue* cmdval;
			m_CurrentCommand->Type()->Values<CommandValue>(Types::ValueSet::Command, [&attributes, &cmdval](CommandValue* value){
				if (value->CanFitSize(value->GetValueSize(attributes))) {
					cmdval = value;
					return true;
				}
				return false;
			});
			m_Xlation = m_Build.AddSymbol(cmdval->GetTranslation());
			m_Xlation->SetAttributes(Types::DataSourceID::Command, attributes);
			return state_neutral;
		}
		States Parser::Parse_Command_Args() {
			/*if (IsCommandParsing() && !AreCommandArgsParsed()) {
				if (m_CommandArgIt->IsReturn()) {
					auto& vars = m_Build.GetScript().GetLScript()->GetVariables();
					if (auto var = vars.Find(name)) {
						// TODO: something
					}
					//else BREAK();
				}
				else {
					auto type = m_CommandArgIt->GetType();
					auto val_vec = type->GetValueTypes<>(Types::ValueSet::Text);
					if (val_vec.size()) {
						Types::Value::Shared val;
						for (auto v : val_vec) {
							auto stringval = std::static_pointer_cast<Types::StringValue>(v);
							auto translation = stringval->GetTranslation();
							ASSERT(translation);
						}

						auto symbol = Tokens::CreateToken<Tokens::String::Value<Types::Value::Shared>>(val, name);
						m_TokenIt->GetSymbol() = symbol;
						break;
					}
				}
			}*/
			//m_CommandArgIt->GetType()->GetMatchLevel();
			return state_parsing_command_args;
		}
		States Parser::Parse_Operator() {
			if (m_ActiveState == state_parsing_variable) {
				m_ActiveState = state_neutral;
				++m_TokenIt;
				
				if (auto op = m_CurrentOperator->GetUnaryOperation(m_Variable->Ptr(), false)) {
					m_OperationParseState.HoldPostUnary(op, m_Variable);
				}
				else m_OperationParseState.HoldLHS(m_CurrentOperator, m_Variable);
 				m_ActiveState = state_parsing_operator;
				return state_neutral;
			}
			else if (m_ActiveState == state_parsing_operator) {
				if (!m_OperationParseState.IsInChain())
					BREAK();

				++m_TokenIt;
				return state_neutral;
			}
			else if (m_OperatorTokenIt == m_TokenIt) {
				++m_TokenIt;
				m_OperationParseState.StartWithOperator(m_CurrentOperator);
				m_ActiveState = state_parsing_operator;
				return state_neutral;
			}
			return state_parsing_operator;
		}
		States Parser::Parse_Subscript() {
			if (!IsSubscriptDelimiterClosing(m_TokenIt->GetToken()))
				BREAK();
			return state_parsing_subscript;
		}
		void Parser::Parse() {
			States newstate = m_ParseState;
			do {
				static States(Parser::*funcs[States::max_state])() = {
					&Parser::Parse_Neutral, &Parser::Parse_Type, &Parser::Parse_Command, &Parser::Parse_Operator,
					&Parser::Parse_Number, &Parser::Parse_String, &Parser::Parse_Label, &Parser::Parse_Variable,

					&Parser::Parse_Subscript,
					&Parser::Parse_Type_Varlist,
					&Parser::Parse_Command_Args,
				};
				newstate = (this->*funcs[m_ParseState = newstate])();
			} while (newstate != m_ParseState && newstate != state_neutral);
			m_ParseState = newstate;
			return;


			/*auto& types = m_Build.GetTypes();
			auto ptr = *m_TokenIt;
			auto type = ptr->GetToken()->GetType<Tokens::Type>();
			bool newline = false;
			bool just_found_command = false;
			Types::ValueSet val_type = Types::ValueSet::INVALID;

			switch (type) {
			case Tokens::Type::Label: {
				auto& token = ptr->GetToken()->Get<Tokens::Label::Info>();
				auto label = token.GetValue<Tokens::Label::Parameter::LabelValue>();
				
				if (!m_OnNewLine) {
					SendError(Error::label_on_line, label);
				}

				val_type = Types::ValueSet::Label;
				
				auto scriptLabel = m_Build.GetScriptLabel(label);
				if (scriptLabel) {
					AddLabel(scriptLabel, m_TokenIt);
					m_LabelTokens.emplace_back(m_TokenIt);
				}
				//m_UnusedLabels
				break;
			}
			case Tokens::Type::Identifier: {
				Commands::Vector vec;
				auto& token = ptr->GetToken()->Get<Tokens::Identifier::Info<>>();
				auto range = token.GetValue<Tokens::Identifier::ScriptRange>();
				auto name = range.Format();

				if (IsCommandParsing() && !AreCommandArgsParsed()) {
					if (m_CommandArgIt->IsReturn()) {
					}
					else {
						auto type = m_CommandArgIt->GetType();
						auto val_vec = type->GetValueTypes<>(Types::ValueSet::Text);
						if (val_vec.size()) {
							Types::Value* val;
							for (auto v : val_vec) {
								auto stringval = static_cast<Types::TextValue*>(v);
								auto translation = stringval->GetTranslation();
								ASSERT(translation);
							}

							auto symbol = CreateSymbol<Tokens::String::Value<Types::Value*>>(val, name);
							m_TokenIt->GetSymbol() = symbol;

							val_type = Types::ValueSet::Text;
							break;
						}
					}
				}

				if (auto type = GetType(name)) {
				}
				else if (auto ptr = m_Build.GetScriptLabel(name)) {
					// this is a label pointer!
					m_Jumps.emplace_back(ptr->Ptr(), m_TokenIt);

					auto tok = CreateToken<Tokens::Label::Info>(Tokens::Type::LabelRef, range, ptr->Ptr());
					m_TokenIt.Get()->SetToken(tok);
					m_TokenIt.Get()->SetSymbol(CreateSymbol<Tokens::Label::Jump>(tok));

					AddLabelRef(ptr, m_TokenIt);
				}
				else if (m_Commands.FindCommands(name, vec) > 0) {
					// make a token and store it
					if (vec.size() == 1)
						m_TokenIt.Get()->SetToken(CreateToken<Tokens::Command::Info<Command>>(Tokens::Type::Command, range, vec[0]));
					else
						m_TokenIt.Get()->SetToken(CreateToken<Tokens::Command::OverloadInfo<Command>>(Tokens::Type::CommandOverload, range, vec));
						//m_TokenIt.Get().SetToken(ScriptsTokens::MakeShared < Tokens::Command::OverloadInfo<Command> >(range.Begin(), Tokens::Type::CommandOverload, range, vec));

					if (ParseCommandOverloads(vec)) {
						BeginCommandParsing();
						just_found_command = true;
					}
				}
				else if (false /*check variables /) {

				}
				else if (IsCommandParsing()/* && m_CommandArgIt->GetType().IsCompatible()/) {

				}
				else {
					SendError(Error::invalid_identifier, range);
				}

				val_type = Types::ValueSet::Label;
				
				break;
			}
			case Tokens::Type::String: {
				auto& token = ptr->GetToken()->Get<Tokens::String::Info>();
				auto range = token.GetValue<0>();
				auto string = range.Format();

				

				val_type = Types::ValueSet::Text;
				break;
			}
			case Tokens::Type::Number: {
				auto type = Tokens::Number::GetValueType(*ptr->GetToken());
				bool is_int = type != Numbers::Float;
				Numbers::IntegerType int_value;
				Numbers::FloatType float_value;
				
				size_t size;
				if (is_int) {
					auto info = ptr->GetToken()->Get<Tokens::Number::Info<Numbers::IntegerType>>();
					int_value = info.GetValue<Tokens::Number::Parameter::NumberValue>();
					size = CountBitOccupation(int_value.GetValue<Numbers::IntegerType::MaxType>());
				}
				else {
					auto info = ptr->GetToken()->Get<Tokens::Number::Info<Numbers::FloatType>>();
					float_value = info.GetValue<Tokens::Number::Parameter::NumberValue>();
					size = 32;
				}
				
				// 
				Types::Value* best_value_match;
				size_t smallest_fitting_value_size = 0;

				// Contain compatible Value's
				std::vector<Types::Value*> vec;
				// Number of compatible Value's
				size_t n;
				if (IsCommandParsing() && !AreCommandArgsParsed()) {
					auto type = m_CommandArgIt->GetType();
					n = types.GetValues(Types::ValueSet::Number, size, vec, [&best_value_match, &smallest_fitting_value_size, is_int, &vec](Types::Value* value){
						// Keep this value?
						auto& num_value = value->Extend<Types::NumberValue>();
						if (num_value.GetNumberType() != (is_int ? Types::NumberValueType::Integer : Types::NumberValueType::Float))
							return false;

						// Oh, is this the only one we need?
						auto ntype = value->GetType();
						if (!best_value_match || smallest_fitting_value_size > value->GetSize()) {
							best_value_match = value;
							smallest_fitting_value_size = value->GetSize();
						}
						return true;
					});

					// The value isn't compatible with this command argument
					if (!n) {
						BREAK();
					}
				}
				else
				{
					// TODO: figure out what to keep
					n = types.GetValues(Types::ValueSet::Number, size, vec, [this, is_int, &vec](Types::Value* value){
						// Keep this value?
						auto& num_value = value->Extend<Types::NumberValue>();
						if (num_value.GetNumberType() != (is_int ? Types::NumberValueType::Integer : Types::NumberValueType::Float))
							return false;
						return true;
					});
				}

				// No values? Poopy...
				if (!n) {
					SendError(Error::unsupported_value_type);
				}

				// Do we have a chosen Value?
				if (best_value_match) {
					// Now create a token for the value itself
					auto number_val = static_cast<Types::NumberValue*>(best_value_match);
					auto val_symbol = is_int
						? CreateSymbol<Tokens::Number::Value<Types::NumberValue*>>(number_val, *ptr->GetToken<Tokens::Number::Info<Numbers::IntegerType>>())
						: CreateSymbol<Tokens::Number::Value<Types::NumberValue*>>(number_val, *ptr->GetToken<Tokens::Number::Info<Numbers::FloatType>>());

					// Store it back in the script token
					m_TokenIt.Get()->GetSymbol() = val_symbol;
				}
				else {
					// TODO: something
				}
				
				// Victory!
				val_type = Types::ValueSet::Number;
				break;
			}
			case Tokens::Type::Character: {
				auto& token = ptr->GetToken()->Get<Tokens::Character::Info<Character>>();
				switch (auto ch = token.GetValue<Tokens::Character::Parameter::CharacterValue>()) {
				case Character::EOL:
					if (IsCommandParsing()) {
						if (!AreCommandArgsParsed()) {
							BREAK();
						}
						else {
							m_CommandTokenMap.emplace(m_CurrentCommand->GetName(), m_CommandTokenIt);
							FinishCommandParsing();
						}
					}
					newline = true;
					break;
				default:
					SendError(Error::invalid_character, ch);
					break;
				}
				break;
			}
			case Tokens::Type::Delimiter: {
				auto& token = ptr->GetToken()->Get<Tokens::Delimiter::Info<Delimiter>>();

				/*switch (auto type = token.GetValue<Tokens::Delimiter::DelimiterType>()) {
				case Delimiter::BeginScope:
					type.Open();
					break;
				case Delimiter::EndScope:
					type.Close();
					break;
				}*
				break;
			}
			case Tokens::Type::Command: {
				BREAK();			// this shouldn't happen
				++m_TokenIt;
				break;
			}
			case Tokens::Type::Operator: {
				auto& token = ptr->GetToken()->Get<Tokens::Operator::Info<Operators::Type>>();
				auto type = token.GetValue<Tokens::Operator::Parameter::OperatorType>();
				auto rg = token.GetValue<Tokens::Operator::Parameter::ScriptRange>();
				rg.Begin();
				break;
			}
			default:
				
				break;
			}

			if (just_found_command) {
				//auto tok = m_TokenIt.Get().GetToken<Tokens::Command::Info<Command>>();
				//auto cmd = tok->GetValue<Tokens::Command::CommandType>();
				m_CommandTokens.emplace_back(m_CommandTokenIt);
			}
			else if (IsCommandParsing()) {
				if (!AreCommandArgsParsed()) {
					if (val_type != Types::ValueSet::INVALID) {
						switch (val_type) {
						case Types::ValueSet::Null:
							break;
						case Types::ValueSet::Text: {
							break;
						}
						}

						m_CommandArgTokens.emplace_back(m_TokenIt.Get());
					}
					else BREAK();
					NextCommandArg();
				}
			}

			m_OnNewLine = newline;
			++m_TokenIt;*/
		}
		void Parser::Reset()
		{ }
		bool Parser::IsFinished() const {
			return m_State == finished;
		}
		bool Parser::IsRunning() const {
			return m_State == init || m_State == parsing;
		}
		size_t Parser::GetNumTokens() const {
			return m_Tokens.Size();
		}
		size_t Parser::GetCurrentToken() const {
			return m_TokenIt.GetIndex();
		}
		Scripts::Token Parser::GetToken() const {
			return m_TokenIt.Get();
		}

		void Parser::SendError(Error type)
		{
			if (m_State == overloading) {
				FailCommandOverload();
			}
			else {
				// send
				std::vector<std::string> params;
				m_Task(Event::Error, Basic::Error(type), params);
			}
		}
		template<typename First, typename... Args>
		void Parser::SendError(Error type, First&& first, Args&&... args)
		{
			if (m_State == overloading) {
				FailCommandOverload();
			}
			else {
				// storage for error parameters
				std::vector<std::string> params;
				// format the error parameters to the vector
				m_Engine.Format(params, first, args...);
				// send
				m_Task(Event::Error, Basic::Error(type), params);
			}
		}
	}
}