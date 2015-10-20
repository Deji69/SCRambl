#include "stdafx.h"
#include "Parser.h"
#include "Standard.h"
#include "Scripts.h"
#include "Preprocessor.h"
#include "TokensB.h"
#include "Tokens.h"
#include "Engine.h"
#include "Tasks.h"
#include <cctype>

using namespace SCRambl;
using namespace SCRambl::Parsing;
using namespace SCRambl::Tokens;

// SCRambl::Parser::Parser
Parser::Parser(Task& task, Engine& engine, Build& build) :
	m_Task(task), m_Engine(engine), m_Build(build), m_State(init),
	m_Tokens(build.GetScript().GetTokens()),
	m_Commands(build.GetCommands()),
	m_Types(build.GetTypes()),
	m_OperationParseState_(*this)
{ }
void Parser::Init() {
	m_TokenIt = m_Tokens.Begin();
	m_Task.Event<event_begin>();
	m_BuildConfig = m_Engine.GetBuildConfig();
	m_State = parsing;
	m_OnNewLine = true;
	m_ParsingCommandArgs = false;
	m_EndOfCommandArgs = false;
	m_Conditional = false;
	m_SizeCount = 0;
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
			m_Task.Event<event_finish>();
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
	if (m_CurrentCommand->NumParams()) {
		m_CommandArgIt = m_CurrentCommand->BeginArg();
		m_ParsingCommandArgs = true;
	}
}
bool Parser::GetDelimitedArrayIntegerConstant(size_t& i) {
	bool b = false;
	auto next = PeekToken(Tokens::Type::Delimiter);
	if (next) {
		if (!IsSubscriptDelimiter(next)) {
			BREAK();
		}
		EnterSubscript(next);
		if (next = PeekToken(Tokens::Type::Number, 2)) {
			i = GetIntegerConstant<size_t>(next);
			b = true;
			++m_TokenIt;
		}
		++m_TokenIt;
		auto next = PeekToken(Tokens::Type::Delimiter);
		if (!next || !IsSubscriptDelimiterClosing(next))
			BREAK();
		m_TokenIt += 2;
	}
	return b;
}
Tokens::CommandArgs::Arg* Parser::AddCommandArg(Operand op, Types::Value* val) {
	if (m_ActiveState == state_parsing_command_args) {
		if (m_CommandParseState.AnyParamsLeft()) {
			return m_CommandParseState.AddArg(op, val);
		}
		else m_Task.Event<error_too_many_args>();
	}
	BREAK();
	return nullptr;
}

States Parser::Parse_Neutral_CheckCharacter(IToken* tok) {
	if (IsCharacterEOL(tok)) {
		switch (m_ActiveState) {
		case state_parsing_operator: {
			OperationParseState_::ParseResult result;
			if (m_OperationParseState_.Finish()) {
				m_OperationParseState_.Reset();
			}
			else {
				BREAK();
			}
			break;
		}
		case state_parsing_command:
		case state_parsing_command_args:
			if (!m_CommandParseState.AreParamsSatisfied()) {
				BREAK();
			}
			// figure out the translation data size of the arg list
			{
				size_t size = m_Xlation->GetTranslation()->GetSize(*m_Xlation);
				for (auto arg : m_CommandParseState.args) {
					auto val = arg.second;
					auto n = val->GetTranslation()->GetSize(FormArgumentXlate(*m_Xlation, arg));
					size += n;
					arg.first.SetSize(size);
				}

				m_SizeCount += BitsToBytes(size);
			}
			m_CommandTokenMap.emplace(m_CommandParseState.command->Name(), m_CommandTokenIt);
			auto token = CreateToken<Tokens::CommandArgs::Info>(Tokens::Type::ArgList, m_CommandParseState.args);
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

	else if (auto label = m_Build.GetScriptLabel(name)) {
		m_Label = label;
		m_LabelTokenIt = m_TokenIt;
		AddLabelRef(label, m_TokenIt);
		return state_parsing_label;
	}
	else if (m_ExtraCommands.FindCommands(name, vec) > 0 || m_Commands.FindCommands(name, vec) > 0) {
		// make a token and store it
		if (vec.size() == 1)
			m_TokenIt->SetToken(new Tokens::Command::Info(Tokens::Type::Command, range, vec[0]));
		else
			m_TokenIt->SetToken(new Tokens::Command::OverloadInfo(Tokens::Type::CommandOverload, range, vec));

		if (ParseCommandOverloads(vec)) {
			BeginCommandParsing();
			m_CommandTokens.emplace_back(m_TokenIt);
		}
		else BREAK();
		return state_parsing_command;
	}
	else if (auto var = m_Build.GetScriptVariable(name)) {
		m_Variable = var;
		m_VariableTokenIt = m_TokenIt;
		return state_parsing_variable;
	}
	else if (IsCommandParsing()/* && m_CommandArgIt->GetType().IsCompatible()*/) {
		auto type = m_CommandParseState.commandArgIt->GetType();
		if (type->HasValueType(Types::ValueSet::Label)) {
			m_Label = m_Build.AddScriptLabel(name, m_SizeCount);
			m_LabelTokenIt = m_TokenIt;
			AddLabelRef(label, m_TokenIt);
			return state_parsing_label;
		}
		else m_Task.Event<error_invalid_identifier>(range);
	}
	else {
		m_Task.Event<error_invalid_identifier>(range);
	}
	return state_neutral;
}
States Parser::Parse_Neutral_CheckLabel(IToken* tok) {
	auto& token = tok->Get<Tokens::Label::Info>();
	auto range = token.GetValue<Tokens::Label::ScriptRange>();
	auto name = range.Format();
	ScriptLabel* label = nullptr;
	if (label = m_Build.GetScriptLabel(name)) (*label)->SetOffset(m_SizeCount);
	else label = m_Build.AddScriptLabel(name, m_SizeCount);
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
			m_Build.CloseVarScope();
			m_Build.CloseLabelScope();
		}
		else if (IsScopeDelimiter(tok)) {
			m_Build.OpenLabelScope();
			m_Build.OpenVarScope();
		}
		else BREAK();
	}
	return state_neutral;
}
States Parser::Parse_Neutral_CheckOperator(IToken* tok) {
	if (auto op = GetOperator(tok)) {
		auto pr = m_Build.GetOperators().Get(op->Name(), m_Conditional);
		m_OperatorType = pr.second;
		if (m_CurrentOperator = pr.first) {
			m_OperatorTokenIt = m_TokenIt;
			if (m_CurrentOperator->IsSign()) {
				if (m_CurrentOperator->IsPositive() || m_CurrentOperator->IsNegative()) {
					if (PeekToken(Tokens::Type::Number) && (m_ActiveState == state_neutral || m_ActiveState == state_parsing_command_args)) {
						++m_TokenIt;
						auto state = Parse_Neutral_CheckNumber(*m_TokenIt);
						if (m_CurrentOperator->IsNegative())
							m_NumberParseState.Negate();
						return state;
					}
				}
			}
			return state_parsing_operator;
		}
	}
	m_Task.Event<error_invalid_operator>(GetOperatorRange(tok));
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
	auto type = m_TokenIt->GetToken()->GetType<Tokens::Type>();

	switch (type) {
	case Tokens::Type::Character:
		new_state = Parse_Neutral_CheckCharacter(*m_TokenIt);
		break;
	case Tokens::Type::Identifier:
		new_state = Parse_Neutral_CheckIdentifier(*m_TokenIt);
		break;
	case Tokens::Type::Label:
		new_state = Parse_Neutral_CheckLabel(*m_TokenIt);
		break;
	case Tokens::Type::Delimiter:
		new_state = Parse_Neutral_CheckDelimiter(*m_TokenIt);
		break;
	case Tokens::Type::Operator:
		new_state = Parse_Neutral_CheckOperator(*m_TokenIt);
		break;
	case Tokens::Type::Number:
		new_state = Parse_Neutral_CheckNumber(*m_TokenIt);
		break;
	case Tokens::Type::String:
		new_state = Parse_Neutral_CheckString(*m_TokenIt);
		break;
	}
	if (new_state == state_neutral)
		++m_TokenIt;
	return new_state;
}
States Parser::Parse_Type() {
	++m_TokenIt;
	auto& type = m_TypeParseState.type;

	// check for special declaration types such as commands, labels or constants
	if (type->HasValueType(Types::ValueSet::Command)) {
		return state_parsing_type_command;
	}
	return state_parsing_type_varlist;
}
States Parser::Parse_Number() {
	size_t size;
	Operand operand;
	if (!m_NumberParseState.IsFloat()) {
		auto val = m_NumberParseState.GetInt();
		operand = { val, m_Engine.Format(val) };
		size = val.Size();
	}
	else {
		auto val = m_NumberParseState.GetFloat();
		operand = { val, m_Engine.Format(val) };
		size = val.Size();
	}

	auto value = GetBestValue(Types::ValueSet::Number, size);
	if (!value) BREAK();
	auto& type = value->GetType();
		
	if (m_ActiveState == state_parsing_command_args) {
		if (!AddCommandArg(operand, value)) BREAK();
		++m_TokenIt;
		return state_neutral;
	}
	else if (m_ActiveState == state_parsing_operator) {
		m_OperationParseState_.MeetValue(operand, type);
		++m_TokenIt;
		return state_neutral;
	}
	else BREAK();
	return state_parsing_number;
}
States Parser::Parse_String() {
	if (m_ActiveState == state_parsing_command_args) {
		auto str = GetTextString(m_TokenIt->GetToken());
		auto value = GetBestValue(Types::ValueSet::Text, str.size());
		if (!value) BREAK();
		if (!AddCommandArg(str, value)) BREAK();
		++m_TokenIt;
		return state_neutral;
	}
	else if (m_ActiveState == state_parsing_operator) {
		BREAK();
	}
	else BREAK();
	return state_parsing_string;
}
States Parser::Parse_Label() {
	if (m_ActiveState == state_parsing_command_args) {
		auto size = CountBitOccupation(m_Label->Get().Offset());
		auto& label = *m_Label;
		auto value = AllFittingValues<Types::LabelValue>(Types::ValueSet::Label, size, [&label](Types::LabelValue* value){
			return value->IsGlobal() == label->IsGlobal();
		});
		if (!value) BREAK();
		AddCommandArg(&*m_Label, value);
		++m_TokenIt;
		return state_neutral;
	}
	return state_parsing_label;
}
States Parser::Parse_Variable() {
	// If we're not parsing command args, parse an operation
	if (m_ActiveState != state_parsing_command_args) {
		m_OperationParseState_.MeetVariable(m_Variable);
		++m_TokenIt;
		return state_neutral;
	}
	else {
		// Add command arg
		auto value = GetBestVarValue(m_Variable);
		if (!value) BREAK();
		AddCommandArg(m_Variable, value);
		++m_TokenIt;
		return state_neutral;
	}
	/*else if (m_ActiveState == state_parsing_command_args) {
		auto size = CountBitOccupation((*m_Variable)->Index());
		auto& var = *m_Variable;
		auto value = AllFittingValues<Types::VariableValue>(Types::ValueSet::Variable, size, [&var](Types::VariableValue* value){
			return value->IsGlobal() == var->IsGlobal();
		});
		if (!value) BREAK();
		AddCommandArg(m_Variable, value);
		++m_TokenIt;
		return state_neutral;
	}
	else if (m_ActiveState == state_neutral) {
		// lhs of a var= operation?
		++m_TokenIt;
		m_ActiveState = state_parsing_variable;
		m_OperationParseState.StartWithVariable(m_Variable);
		return state_neutral;
	}*/
	return state_parsing_variable;
}
States Parser::Parse_Type_Varlist() {
	// end of var declarations?
	if (IsEOLReached()) {
		return state_neutral;
	}
	else if (IsCharacter(m_TokenIt->GetToken())) {
		m_Task.Event<error_invalid_character>(GetCharacterValue(m_TokenIt->GetToken()));
		return state_neutral;
	}

	// expect an identifier for a var name
	auto type = GetCurrentTokenType();
	if (type != Tokens::Type::Identifier) {
		m_Task.Event<error_expected_identifier>();
		BREAK();
	}
	else {
		// collect var declaration info
		auto name = GetIdentifierName(m_TokenIt->GetToken());
		size_t array_size = 0;
		GetDelimitedArrayIntegerConstant(array_size);

		auto var = m_Build.AddScriptVariable(name, m_TypeParseState.type, array_size);
		if (!var) {
			BREAK();
		}
	}
	++m_TokenIt;
	return state_parsing_type_varlist;
}
States Parser::Parse_Type_CommandDef() {
	if (IsEOLReached()) {
		return state_neutral;
	}
	else if (!IsTokenType(m_TokenIt->GetToken(), Tokens::Type::Identifier)) {
		m_Task.Event<error_expected_identifier>();
		++m_TokenIt;
	}
	else {
		auto name = GetIdentifierName(m_TokenIt->GetToken());
		if (name.empty()) m_Task.Event<error_expected_identifier>();
		else {
			bool cond = false;
			auto chartok = PeekToken(Tokens::Type::Character);
			if (chartok && IsConditionPunctuator(chartok)) {
				cond = true;
				++m_TokenIt;
				chartok = PeekToken(Tokens::Type::Character);
			}
			if (!chartok || !IsColonPunctuator(chartok))
				m_Task.Event<error_expected_colon_punctuator>();
			++m_TokenIt;
					
			auto id = GetTokenString(PeekToken());
			if (!id.empty()) {
				auto command = m_ExtraCommands.AddCommand(name, id, m_TypeParseState.type);
				++m_TokenIt;

				while (auto idtok = PeekToken()) {
					if (IsEOLReached()) break;

					// check for assignment operator as 'return' flag for arg
					bool isret = false;
					if (GetTokenType(idtok) == Tokens::Type::Operator) {
						auto op = Tokens::Operator::GetOperator<Operators::OperatorRef>(*idtok);
						if (!op->IsAssignment()) break;
						isret = true;

						auto optok = PeekToken(Tokens::Type::Identifier, 1);
						if (!optok) break;
						std::swap(idtok, optok);
						++m_TokenIt;
					}
					if (GetTokenType(idtok) != Tokens::Type::Identifier) break;

					// get the type
					auto type = m_Types.GetType(GetTokenString(idtok));
					if (!type) break;
					++m_TokenIt;

					// size-specific value?
					size_t size = 0;
					GetDelimitedArrayIntegerConstant(size);

					// add to command
					command->AddArg(type.Ref(), isret, size);
				}
			}
			else m_Task.Event<error_expected_key_identifier>();
		}
	}
	return state_parsing_type_command;
}
States Parser::Parse_Command() {
	if (m_ActiveState == state_parsing_command_args) {
		BREAK();
	}

	m_CommandParseState.Begin(m_CurrentCommand, m_CurrentCommand->BeginArg());
	auto attributes = m_CurrentCommand->GetAttributes();
	CommandValue* cmdval = nullptr;
	m_CurrentCommand->Type()->Values<CommandValue>(Types::ValueSet::Command, [&attributes, &cmdval](CommandValue* value){
		if (value->CanFitSize(value->GetValueSize(attributes))) {
			cmdval = value;
			return true;
		}
		return false;
	});
	if (cmdval) {
		m_Xlation = m_Build.AddSymbol(cmdval->GetTranslation());
		m_Xlation->SetAttributes(Types::DataSourceID::Command, attributes);
		++m_TokenIt;
		return m_CurrentCommand->NumParams() > 0 ? state_parsing_command_args : state_neutral;
	}
	else m_Task.Event<error_invalid_command>();
	return state_neutral;
}
States Parser::Parse_Command_Arglist() {
	m_ActiveState = state_parsing_command_args;
	return state_neutral;
}
States Parser::Parse_Operator() {
	if (m_OperationParseState_.MeetOperator(m_CurrentOperator)) {
		m_ActiveState = state_parsing_operator;
		++m_TokenIt;
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
			&Parser::Parse_Type_Varlist, &Parser::Parse_Type_CommandDef,
			&Parser::Parse_Command_Arglist
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
	return m_TokenIt.Index();
}
Token Parser::GetToken() const {
	return *m_TokenIt;
}
IToken* Parser::PeekToken(Tokens::Type type, size_t off) {
	auto it = m_TokenIt + off;
	if (it != m_Tokens.End()) {
		auto tok = it->GetToken();
		auto ty = tok->GetType<Tokens::Type>();
		if (type == Tokens::Type::None || type == ty) {
			return tok;
		}
	}
	return nullptr;
}

// SCRambl::Parser::Task
size_t Task::GetProgressCurrent() const { return GetCurrentToken(); }
size_t Task::GetProgressTotal() const { return GetNumTokens(); }
Token Task::GetToken() const { return Parser::GetToken(); }
bool Task::IsRunning() const { return Parser::IsRunning(); }
bool Task::IsTaskFinished() const { return Parser::IsFinished(); }
void Task::RunTask() { Parser::Run(); }
void Task::ResetTask() { Parser::Reset(); }
Task::Task(Engine& engine, Build* build) : TaskSystem::Task(build),
Parser(*this, engine, *build), m_Engine(engine)
{ }

Types::Xlation Parsing::FormArgumentXlate(const Types::Xlation& xlate, const Tokens::CommandArgs::Arg& arg) {
	Types::Xlation r = xlate;
	r.SetAttributes(Types::DataSourceID::Number, arg.first.GetNumberAttributes());
	r.SetAttributes(Types::DataSourceID::Text, arg.first.GetTextAttributes());
	r.SetAttributes(Types::DataSourceID::Variable, arg.first.GetVariableAttributes());
	r.SetAttributes(Types::DataSourceID::Label, arg.first.GetLabelAttributes());
	return r;
}

std::map<Error::ID, std::string> Error::s_map = {
	{ Error::expected_colon_punctuator, "expected colon punctuator" },
	{ Error::expected_identifier, "expected identifier" },
	{ Error::expected_integer_constant, "expected integer constant" },
	{ Error::expected_key_identifier, "expected key identifier" },
	{ Error::invalid_character, "invalid character" },
	{ Error::invalid_command, "invalid command" },
	{ Error::invalid_identifier, "invalid identifier" },
	{ Error::invalid_operator, "invalid operator" },
	{ Error::label_on_line, "label on line" },
	{ Error::too_many_args, "too many args" },
	{ Error::unsupported_value_type, "unsupported value type" },
};