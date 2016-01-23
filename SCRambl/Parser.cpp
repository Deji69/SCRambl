#include "stdafx.h"
#include "Parser.h"
#include "Standard.h"
#include "Scripts.h"
#include "Preprocessor.h"
#include "TokensB.h"
#include "Tokens.h"
#include "Engine.h"
#include "Tasks.h"
#include "Numbers.h"
#include <cctype>

using namespace SCRambl;
using namespace SCRambl::Parsing;
using namespace SCRambl::Tokens;

// SCRambl::Parser::Parser
Parser::Parser(Task& task, Engine& engine, Build& build) :
	m_Task(task), m_Engine(engine), m_Build(build), m_State(init),
	m_Tokens(build.GetScript().GetTokens()),
	m_Commands(build.GetCommands()),
	m_Types(build.GetTypes())
{
	m_OperationParseStates.emplace_back(*this);
}
void Parser::Init() {
	m_TokenIt = m_Tokens.Begin();
	m_Task.Event<event_begin>();
	m_BuildConfig = m_Engine.GetBuildConfig();
	m_ParseVars.clear();
	m_UsedParseVars.clear();
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
			i = GetIntegerConstant<Numbers::IntegerType::ULongType>(next);
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
ScriptVariable* Parser::CreateParseVar(const Types::Type* type, size_t array_size) {
	ScriptVariable* var = nullptr;

	if (!m_ParseVars.empty() && m_UsedParseVars.size() < m_ParseVars.size()) {
		size_t i = 0;
		for (; i < m_ParseVars.size(); ++i) {
			if (m_UsedParseVars.find(m_ParseVars[i]) == m_UsedParseVars.end())
				break;
		}
		if (i < m_ParseVars.size()) {
			var = m_ParseVars[i];
		}
	}

	if (!var) {
		if (!type->HasValueType(array_size ? Types::ValueSet::Array : Types::ValueSet::Variable)) {
			auto types = array_size ? type->ArrayTypes() : type->VarTypes();
			const Types::Type* best = nullptr;
			bool preferLocal = m_Build.GetVariables().LocalDepth() > 0;
			bool bestIsLocal = false;
			if (!types.empty()) {
				Types::MatchLevel bestLevel = Types::MatchLevel::None;

				for (auto id : types) {
					auto& newtype = m_Types.GetType(id);
					auto matchLevel = newtype.Ref()->GetMatchLevel(type);
					bool bestOnScope = false;

					if (best && preferLocal) {
						if (!bestIsLocal && newtype.Ref()->GetVarValue()->GetVarType()->IsScopedVar())
							bestOnScope = true;
					}
					
					switch (matchLevel) {
					case Types::MatchLevel::Loose:
					case Types::MatchLevel::Basic:
						if (bestLevel == Types::MatchLevel::Basic && !bestOnScope)
							continue;
					case Types::MatchLevel::Strict:
						if (bestLevel != Types::MatchLevel::Strict) {
							break;
						}
						else if (bestLevel == matchLevel && bestOnScope)
							break;
					default: continue;
					}

					best = newtype.Ptr();
					bestLevel = matchLevel;
					bestIsLocal = best->IsScopedVar();
				}
			}
			
			if (best) type = best;
			else {
				ASSERT(best);
				m_Task.Event<error_no_var_type_for_subexp>(type);
			}
		}

		std::string name = "__PV" + std::to_string(m_ParseVars.size()) + "_" + type->GetName();
		var = m_Build.AddScriptVariable(name, m_Types.GetType(type->GetID()).Ref(), array_size);
		m_ParseVars.emplace_back(var);
	}

	m_UsedParseVars.emplace(var);
	return var;
}
void Parser::FreeParseVar(ScriptVariable* var) {
	m_UsedParseVars.erase(var);
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
			if (m_OperationParseStates.size() > 1)
				BREAK(); // error: finish yo damn evaluation

			if (m_OperationParseStates.back().Finish()) {
				m_OperationParseStates.back().Reset();
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
		if (IsEvaluationDelimiter(tok)) {
			switch (m_ActiveState) {
			case state_parsing_operator:
				if (!IsEvaluationDelimiterClosing(tok)) {
					if (m_OperationParseStates.back().m_State == OperationParseState_::waitRHS) {
						// enter sub-evaluation
						m_OperationParseStates.emplace_back(*this, true);
					}
					else {
						m_Task.Event<error_expected_rhs_value>();	// error: bad place in the operation for sub-evaluation
					}
				}
				else if (m_OperationParseStates.size() <= 1) {
					m_Task.Event<error_unmatched_closing_delimiter>(Tokens::Delimiter::GetDelimiterType<Delimiter>(*tok));
				}
				else {
					// combine sub-expression with parent expression
					auto& opState = m_OperationParseStates[m_OperationParseStates.size() - 2];
					auto& subexp = m_OperationParseStates.back();
					if (subexp.m_OperandChain.size() == 1 && subexp.m_OperationChain.empty()) {
						opState.MeetValue(subexp.m_OperandChain[0].first, subexp.m_OperandChain[0].second);
					}
					else {
						if (!subexp.Finish()) {
							BREAK();
						}

						if (subexp.m_EvalVar) opState.MeetVariable(subexp.m_EvalVar);
						/*for (auto& op : m_OperationParseStates.back().m_OperandChain)  {
							opState.m_OperandChain.emplace_back(op);
						}
						for (auto& op : m_OperationParseStates.back().m_OperationChain)  {
							opState.m_OperationChain.emplace_back(op);
						}*/
					}

					m_OperationParseStates.pop_back();
				}
				break;
			}
		}
		else if (IsScopeDelimiterClosing(tok)) {
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
		m_OperationParseStates.back().MeetValue(operand, type);
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
		m_OperationParseStates.back().MeetVariable(m_Variable);
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
	if (m_OperationParseStates.back().MeetOperator(m_CurrentOperator)) {
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
	{ Error::expected_rhs_value, "expected rhs value" },
	{ Error::invalid_character, "invalid character" },
	{ Error::invalid_command, "invalid command" },
	{ Error::invalid_identifier, "invalid identifier" },
	{ Error::invalid_operator, "invalid operator" },
	{ Error::label_on_line, "label on line" },
	{ Error::too_many_args, "too many args" },
	{ Error::unmatched_closing_delimiter, "unmatched closing delimiter" },
	{ Error::unsupported_value_type, "unsupported value type" },
};