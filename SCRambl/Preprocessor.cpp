#include "stdafx.h"
#include "Standard.h"
#include "Preprocessor.h"
#include "Lexer.h"

using namespace SCRambl;
using namespace SCRambl::Preprocessing;

bool DoesDirectiveIgnoreSourceControl(Directive::Type dir) {
	switch (dir) {
	case Directive::IF:
	case Directive::IFDEF:
	case Directive::IFNDEF:
	case Directive::ELIF:
	case Directive::ELSE:
	case Directive::ENDIF:
		return true;
	}
	return false;
}

Preprocessor::Preprocessor(Task& task, Engine& engine, Build& build) :
	m_State(init), m_Task(task),
	m_Engine(engine), m_Build(build),
	m_Tokens(build.GetScript().GetTokens()),
	m_Lexer(),
	m_OperatorScanner(m_Operators),
	m_Information(m_CodePos),
	m_Commands(build.GetCommands()),
	m_ParserOperators(build.GetOperators().GetTable()),
	m_ParserOperatorScanner(m_ParserOperators)
{
	Reset();

	// we need token scanners!
	//m_Lexer.AddTokenScanner(TokenType::None, m_WhitespaceScanner);
	m_Lexer.AddTokenScanner(TokenType::BlockComment, m_BlockCommentScanner);
	m_Lexer.AddTokenScanner(TokenType::Comment, m_CommentScanner);
	m_Lexer.AddTokenScanner(TokenType::Directive, m_DirectiveScanner);
	m_Lexer.AddTokenScanner(TokenType::String, m_StringLiteralScanner);
	m_Lexer.AddTokenScanner(TokenType::Label, m_LabelScanner);
	m_Lexer.AddTokenScanner(TokenType::Identifier, m_IdentifierScanner);
	m_Lexer.AddTokenScanner(TokenType::Number, m_NumericScanner);
	m_Lexer.AddTokenScanner(TokenType::Operator, m_OperatorScanner);
	m_Lexer.AddTokenScanner(TokenType::ParseOperator, m_ParserOperatorScanner);
	m_OperatorScanner.Disable(); // only for preprocessor lines

	// map directives
	m_Directives["define"] = Directive::DEFINE;
	m_Directives["elif"] = Directive::ELIF;
	m_Directives["else"] = Directive::ELSE;
	m_Directives["endif"] = Directive::ENDIF;
	m_Directives["if"] = Directive::IF;
	m_Directives["ifdef"] = Directive::IFDEF;
	m_Directives["ifndef"] = Directive::IFNDEF;
	m_Directives["include"] = Directive::INCLUDE;
	m_Directives["undef"] = Directive::UNDEF;

	m_Directives["register_var"] = Directive::REGISTER_VAR;
	m_Directives["register_command"] = Directive::REGISTER_COMMAND;

	// arithmetic operators - add em
	m_Operators.AddOperator("+", Operators::Type::add);
	m_Operators.AddOperator("-", Operators::Type::sub);
	m_Operators.AddOperator("++", Operators::Type::inc);
	m_Operators.AddOperator("--", Operators::Type::dec);
	m_Operators.AddOperator("*", Operators::Type::mult);
	m_Operators.AddOperator("/", Operators::Type::div);
	m_Operators.AddOperator("%", Operators::Type::mod);

	// bitwise operators - add em
	m_Operators.AddOperator("&", Operators::Type::bit_and);
	m_Operators.AddOperator("|", Operators::Type::bit_or);
	m_Operators.AddOperator("^", Operators::Type::bit_xor);
	m_Operators.AddOperator("<<", Operators::Type::bit_shl);
	m_Operators.AddOperator(">>", Operators::Type::bit_shr);

	// comparison operators - add em
	//m_Operators.AddOperator("=", Operator::eq);
	m_Operators.AddOperator(">", Operators::Type::gt);
	m_Operators.AddOperator("<", Operators::Type::lt);
	m_Operators.AddOperator("==", Operators::Type::eq);
	m_Operators.AddOperator("!=", Operators::Type::neq);
	m_Operators.AddOperator(">=", Operators::Type::geq);
	m_Operators.AddOperator("<=", Operators::Type::leq);

	// logical operators - add em
	m_Operators.AddOperator("!", Operators::Type::not);
	m_Operators.AddOperator("&&", Operators::Type::and);
	m_Operators.AddOperator("||", Operators::Type::or);
	m_Operators.AddOperator("?", Operators::Type::cond);
	m_Operators.AddOperator(":", Operators::Type::condel);

	// formatters for messages - set defaults
	m_Engine.SetFormatter<Numbers::IntegerType>(Numbers::IntegerType::Formatter);
	m_Engine.SetFormatter<Numbers::FloatType>(Numbers::FloatType::Formatter);
	m_Engine.SetFormatter<Scripts::Position>(Scripts::Position::Formatter);
	m_Engine.SetFormatter<Scripts::Range>(Scripts::Range::Formatter);
	m_Engine.SetFormatter<Directive>(Directive::Formatter);
	m_Engine.SetFormatter<Label*>(Label::Formatter);
}
void Preprocessor::Reset() {
	switch (m_State) {
	default:
		// mid-process reset
	case finished:
		// end-process reset
		break;
	case init:
		// begin-process init
		break;
	}

	m_State = init;
}
void Preprocessor::Run() {
	try {
		switch (m_State) {
		case init:
			m_Task.Event<event_begin>();
			m_CodePos = m_Build.GetScript().GetCode();
			m_State = lexing;
			break;
		default:
			RunningState();
			break;
		case finished:
			Reset();
			break;
		}
	}
	catch (...)
	{
		throw;
	}
}
void Preprocessor::RunningState() {
	auto old_state = m_State;

	switch (m_State) {
	default:
	case lexing:
		LexerPhase();
		break;
	case found_comment:
		HandleComment();
		break;
	case found_directive:
		HandleDirective();
		break;
	case found_token:
		HandleToken();
		break;
	}
}
void Preprocessor::HandleToken() {
	auto range = m_Token.Range();
	auto pos = m_Token.Begin();
	m_WasLastTokenEOL = false;

	switch (m_Token) {
	case TokenType::Eol: {
		m_Build.CreateToken<Tokens::Character::Info<Character>>(range, Tokens::Type::Character, pos, Character(Character::Type::EOL));
		break;
	}
	case TokenType::Identifier: {
		m_Build.CreateToken<Tokens::Identifier::Info<>>(range, Tokens::Type::Identifier, m_Token.Range());
		break;
	}
	case TokenType::Number: {
		if (m_NumericScanner.Is<int>())
			m_Build.CreateToken<TokenNumber<Numbers::IntegerType, Numbers::Integer>>(range, range, m_NumericScanner.Get<unsigned long long>());
		else
			m_Build.CreateToken<TokenNumber<Numbers::FloatType, Numbers::Float>>(range, range, m_NumericScanner.Get<float>());
		break;
	}
	case TokenType::Label: {
		auto name = range.Format();
		// TODO: do
		auto label = m_Build.AddScriptLabel(name, pos);
		m_Build.CreateToken<Tokens::Label::Info>(range, Tokens::Type::Label, range, label->Ptr());
		break;
	}
	case TokenType::Operator: {
		m_Build.CreateToken<Tokens::Operator::Info<Operators::Type>>(range, Tokens::Type::Operator, range, m_OperatorScanner.GetOperator());
		break;
	}
	case TokenType::ParseOperator: {
		m_Build.CreateToken<Tokens::Operator::Info<Operators::OperatorRef>>(range, Tokens::Type::Operator, range, m_ParserOperatorScanner.GetOperator());
		break;
	}
	case TokenType::Directive: {
		m_Build.CreateToken<Tokens::Directive::Info>(range, Tokens::Type::Directive, range);
		break;
	}
	case TokenType::String: {
		m_Build.CreateToken<Tokens::String::Info>(range, Tokens::Type::String, range, m_String);
		break;
	}
	default: break;
	}

	m_State = lexing;
}
void Preprocessor::HandleDirective() {
	auto& task = m_Task;
	switch (auto directive = m_Directive) {
	case Directive::DEFINE:
		if (Lex() == Lexing::Result::found_token && m_Token == TokenType::Identifier) {
			Macro::Name name = m_Identifier;

			// skip to the good bit
			while (m_CodePos && m_CodePos->IsIgnorable()) ++m_CodePos;

			// gather up thy symbols
			if (m_CodePos && m_CodePos->GetType() != Symbol::eol) {
				std::set<const Macro*> macrosThatAreNotMacros;
				CodeLine code;

				auto start_pos = m_CodePos;
				while (m_CodePos && m_CodePos->GetType() != Symbol::eol) {
					if (m_CodePos->IsIgnorable()) {
						++m_CodePos;
						continue;
					}
					auto result = m_Lexer.ScanFor(TokenType::Identifier, m_CodePos, m_Token);
					switch (result) {
					case Lexing::Result::still_scanning:
						continue;
					case Lexing::Result::found_nothing:
						break;
					case Lexing::Result::found_token:
						m_Identifier = m_Token.Range().Format();

						if (auto macro = m_Macros.Get(m_Identifier)) {
							if (macrosThatAreNotMacros.find(macro) == macrosThatAreNotMacros.end()) {
								macrosThatAreNotMacros.emplace(macro);
								bool b = m_CodePos == start_pos;
								m_CodePos = m_Build.GetScript().GetCode().Erase(m_Token.Begin(), m_Token.End());
								m_CodePos = m_Build.GetScript().GetCode().Insert(m_CodePos, macro->GetCode());
								if (b) start_pos = m_CodePos;
								continue;
							}
						}
						m_CodePos = m_Token.End();
						continue;
					}
					++m_CodePos;
				}

				m_Macros.Define(name, m_Build.GetScript().GetCode().Copy(start_pos, m_CodePos, code));
			}
			else m_Macros.Define(name);
		}
		break;

	case Directive::UNDEF:
		// make sure we're not handed dirty macro code
		m_DisableMacroExpansionOnce = true;

		if (Lex() == Lexing::Result::found_token && m_Token == TokenType::Identifier)
			m_Macros.Undefine(m_Identifier);
		else
			throw;
		break;

	case Directive::IF:
		PushSourceControl(GetSourceControl() ? ProcessExpression() != 0 : false);
		break;

	case Directive::IFDEF:
		if (Lex() == Lexing::Result::found_token && m_Token == TokenType::Identifier)
			PushSourceControl(GetSourceControl() ? (m_Macros.Get(m_Identifier) != nullptr) : false);
		break;

	case Directive::IFNDEF:
		if (Lex() == Lexing::Result::found_token && m_Token == TokenType::Identifier)
			PushSourceControl(GetSourceControl() ? (m_Macros.Get(m_Identifier) == nullptr) : false);
		break;

	case Directive::ELIF:
		if (!GetSourceControl()) {
			PopSourceControl();
			PushSourceControl(GetSourceControl() ? (ProcessExpression() != 0) : false);
		}
		break;

	case Directive::ELSE:
		InvertSourceControl();
		break;

	case Directive::ENDIF:
		PopSourceControl();
		break;

	case Directive::INCLUDE:
		if (Lex() == Lexing::Result::found_token && m_Token == TokenType::String)
		{
			if (m_Build.GetScript().Include(m_CodePos, m_String))
			{
				m_State = lexing;
				return;
			}
			else m_Task.Event<error_include_failed>(m_String);
		}
		else m_Task.Event<error_dir_expected_file_name>(m_Directive);
		break;

	case Directive::REGISTER_COMMAND:
		if (Lex(
			[this](const LexerToken& tok){ return tok == TokenType::Number && m_NumericScanner.Is<int>(); },
			[&task, &directive](const LexerToken& tok){ task.Event<error_dir_expected_command_id>(directive); }
			)) {
			auto opcode = m_NumericScanner.Get<size_t>();

			if (Lex(TokenType::Identifier, [this](const LexerToken& tok){
				SendError(Error::dir_expected_identifier, m_Directive);
			})) {
				auto command = m_Commands.AddCommand(m_Identifier, opcode, nullptr);
						
				if (Lex() == Lexing::Result::found_token) {
					bool openParen = m_Token == TokenType::OpenParen;
					if (openParen) {
						if (!Lex(TokenType::Identifier, [this](const LexerToken& tok){ SendError(Error::dir_expected_identifier, m_Directive); }))
							break;
					}
					else if (m_Token != TokenType::Identifier)
						SendError(Error::dir_expected_identifier, m_Directive);

					do {
						auto str = m_Token.Range().Format();
						bool isret = false;
						if (str[0] == '=') {
							isret = true;
							str = str.substr(1);
						}
						if (auto type = GetType(str)) {
							command->AddArg(type, isret);
						}
						if (!Lexpect(TokenType::Separator)) break;
					} while (Lex() == Lexing::Result::found_token);

					if (openParen) Lexpect(TokenType::CloseParen);
				}
			}
		}
		break;

	default:
		//BREAK();		// invalid directive - should've alredy reported - now the preprocessor can continue
		break;
	}

	m_State = lexing;
	return;
}
void Preprocessor::HandleComment() {
	// handle it with care by deleting the shit out of it
	m_CodePos = m_Build.GetScript().GetCode().Erase(m_Token.Begin(), m_Token.End());

	// that felt good... add a single space in its place as an extra sign of indignity...
	m_CodePos.Insert(' ');

	m_State = lexing;
}
void Preprocessor::LexerPhase() {
	if (m_State == lexing) {
		if (m_CodePos) {
			if (m_CodePos->IsEOL()) {
				if (!m_WasLastTokenEOL) {
					AddToken<Tokens::Character::Info<Character>>(m_CodePos, Tokens::Type::Character, m_CodePos, Character::EOL);
					m_WasLastTokenEOL = true;
				}
			}
			else if (m_CodePos->IsPunctuator() && m_CodePos->HasGrapheme()) {
				switch (m_CodePos->GetGrapheme()) {
				case Grapheme::colon:
					AddToken<Tokens::Character::Info<Character>>(m_CodePos, Tokens::Type::Character, m_CodePos, Character::Colonnector);
					break;
				case Grapheme::condition:
					AddToken<Tokens::Character::Info<Character>>(m_CodePos, Tokens::Type::Character, m_CodePos, Character::Conditioner);
					break;
				}
			}
			else {
				m_WasLastTokenEOL = false;
				if (m_CodePos->IsDelimiter()) {
					// only for "range" delimiting - these are independent from tokens using scanners (their contents are separate tokens)
					bool open = true;
					bool success = true;
					Delimiter type;

					switch (*m_CodePos) {
					case '<':
						success = OpenDelimiter(m_CodePos, type = Delimiter::Cast);
						break;
					case '>':
						success = CloseDelimiter(m_CodePos, type = Delimiter::Cast);
						open = false;
						break;
					case '[':
						success = OpenDelimiter(m_CodePos, type = Delimiter::Subscript);
						break;
					case ']':
						success = CloseDelimiter(m_CodePos, type = Delimiter::Subscript);
						open = false;
						break;
					case '{':
						success = OpenDelimiter(m_CodePos, type = Delimiter::Scope);
						break;
					case '}':
						success = CloseDelimiter(m_CodePos, type = Delimiter::Scope);
						open = false;
						break;
					default:
						BREAK();
						break;
					}

					if (!success) {
						if (open) SendError(Error::internal_unable_to_allocate_token);
						else SendError(Error::expr_unmatched_closing_delimiter);
						BREAK();
					}
				}
			}
		}

		while (m_CodePos && m_CodePos->IsIgnorable())
			++m_CodePos;
	}

	// ya, we're done here...
	if (!m_CodePos) {
		m_State = finished;
		m_Task.Event<event_finish>();
		return;
	}

	// Hey, Lex!
	switch (Lex()) {
	case Lexing::Result::found_token:
		// handle state-changing token types
		switch (m_Token.GetType()) {
		case TokenType::Directive:
			m_State = found_directive;
			return;

		default:
			m_State = found_token;
			return;
		}

		m_State = lexing;
		return;

	case Lexing::Result::found_nothing:
		++m_CodePos;
		m_State = lexing;
		return;
	}
}
bool Preprocessor::ExpressUnary(Operators::Type op, int& val) {
	switch (op.Get()) {
	default: return false;
	case Operators::Type::not:
		val = !val;
		break;
	case Operators::Type::bit_not:
		val = ~val;
		break;
	case Operators::Type::sub:
		val = -val;
		break;
	case Operators::Type::add:
		val = +val;			// lol
		break;
	}
	return true;
}
int Preprocessor::ProcessExpression(bool paren) {
	// ultimate expression result
	int result = 0;
	// last expression value
	int val = 0;
	// got an expression value? we'll probably expect an operator...
	bool got_val = false;
	// flag for the 'defined' unary operator
	bool defined_operator = false;
	// ignore a set of parentheses after the unary operators that are kind of like functions ('defined()')
	bool ignore_parentheses = false;
	// if there was a logical operator, this is the logical value of the expression preceeding it
	bool log_operand;

	// logical operator
	Operators::Type log_op = Operators::max_operator;
	Operators::Type last_op = Operators::max_operator;
	Operators::Type op = Operators::max_operator;

	// storage of chained (unary operations, operator code range)
	std::stack<std::pair<Operators::Type, Scripts::Range>>	unary_operators;

	// end of the line at the very beginning? that may be a problem...
	if (m_CodePos->IsEOL()) {
		SendError(Error::expected_expression);
	}

	while (!m_CodePos->IsEOL()) {
		// Lex
		if (Lex() == Lexing::Result::found_nothing) {
			// TODO: elaborate
			SendError(Error::expected_expression);
			continue;
		}

		// Do stuff
		switch (m_Token) {
		default:
			// TODO: elaborate
			SendError(Error::expected_expression);
			continue;

		case TokenType::OpenParen:
			// we may be ignoring parentheses to prevent trying to retrieve a macro as a value (for 'defined(MACRO)')
			// yes, we hate that stupid wannabe operator/keyword/function
			if (!ignore_parentheses) {
				val = ProcessExpression(true);
				got_val = true;
			}
			paren = true;
			break;

		case TokenType::CloseParen:
			if (!paren) {
				SendError(Error::expr_unmatched_closing_parenthesis, std::string(")"));
			}
			paren = false;
			if (ignore_parentheses) {
				ignore_parentheses = false;
				got_val = true;
				break;
			}
			return result;

		case TokenType::Number:
			if (defined_operator) {
				SendError(Error::expected_identifier, m_Token.Range());

				// recover
				val = 0;
				defined_operator = false;
				m_DisableMacroExpansion = false;
				if (!paren) {
					ignore_parentheses = false;
					got_val = true;
				}
				break;
			}

			if (got_val) {
				SendError(Error::expr_expected_operator, m_Token.Range());
			}
			else {
				if (m_NumericScanner.Is<float>()) {
					// for recovery we'll simply try to demote the float to an integer and continue
					// errors during preprocessor expressions may be treated later by completely skipping to #endif (even if an #else is present)
					SendError(Error::expr_unexpected_float, m_Token.Range());
				}
						
				val = m_NumericScanner.Get<int>();
				got_val = true;
			}
			break;

		case TokenType::Operator:
			//if (op == Operator::max_operator) throw;
			if (defined_operator) {
				SendError(Error::expected_identifier, m_Token.Range());

				// recover
				val = 0;
				defined_operator = false;
				m_DisableMacroExpansion = false;
				if (!paren) {
					ignore_parentheses = false;
					got_val = true;
				}
				break;
			}

			last_op = op;
			switch (op = m_OperatorScanner.GetOperator()) {
				// Arithmetic
			case Operators::Type::add:			// +
				// if not, do the binary operation instead
				if (!got_val) {
					// this is unary, correct things
					unary_operators.emplace(Operators::Type::add, std::make_pair(m_Token.Begin(), m_Token.End()));
					op = last_op;
				}
				else got_val = false;
				break;

			case Operators::Type::sub:			// -
				// if not, do the binary operation instead
				if (!got_val) {
					// this is unary, correct things
					unary_operators.emplace(Operators::Type::sub, std::make_pair(m_Token.Begin(), m_Token.End()));
					op = last_op;
				}
				else got_val = false;
				break;

			case Operators::Type::mult:		// *
			case Operators::Type::div:			// /
			case Operators::Type::mod:			// %
				if (!got_val) SendError(Error::invalid_unary_operator, m_Token.Range());
				else got_val = false;
				break;

			case Operators::Type::inc:			// ++
			case Operators::Type::dec:			// --
				SendError(Error::expr_invalid_operator, m_Token.Range());
				break;

				// Bitwise
			case Operators::Type::bit_and:		// &
			case Operators::Type::bit_or:		// |
			case Operators::Type::bit_xor:		// ^
			case Operators::Type::bit_shl:		// <<
			case Operators::Type::bit_shr:		// >>
				if (!got_val) SendError(Error::invalid_unary_operator, m_Token.Range());
				else got_val = false;
				break;

			case Operators::Type::bit_not:
				// if not, wait, this IS not!
				if (!got_val) {
					// this is unary, correct things
					unary_operators.emplace(Operators::Type::bit_not, std::make_pair(m_Token.Begin(), m_Token.End()));
					op = last_op;
				}
				break;

				// Comparison
			case Operators::Type::gt:			// >
			case Operators::Type::lt:			// <
			case Operators::Type::geq:			// >=
			case Operators::Type::leq:			// <=
			case Operators::Type::eq:			// ==
			case Operators::Type::neq:			// !=
				if (!got_val) SendError(Error::invalid_unary_operator, m_Token.Range());
				else got_val = false;
				break;

				// Logical
			case Operators::Type::not:			// !
				// if not, wait, THIS IS NOT TOOO!!!
				if (!got_val) {
					// this is unary, correct things
					unary_operators.emplace(Operators::Type::not, std::make_pair(m_Token.Begin(), m_Token.End()));
					op = last_op;
				}
				else SendError(Error::invalid_unary_operator_use, m_Token.Range());
				break;

			case Operators::Type::and:			// &&
			case Operators::Type::or:			// ||
				// if got
				if (got_val) {
					log_operand = result != 0;
					log_op = op;
					got_val = false;
				}
				else SendError(Error::expected_expression);
				break;

			case Operators::Type::cond:		// ?
			case Operators::Type::condel:	// :
				// if (TODO?DO:NOTTODO) that is the question;
				got_val = false;
				break;

			case Operators::Type::max_operator:
				throw;
				break;

			default:
				throw;
				break;
			}

			ASSERT((!got_val || op != Operators::max_operator) && "Premature error in testing phase (unary used AFTER a value)");
			break;

		case TokenType::Identifier:
			if (defined_operator)
			{
				val = m_Macros.Get(m_Identifier) != nullptr;
				got_val = true;
				defined_operator = false;
				m_DisableMacroExpansion = false;

				// if there were no parentheses, quit trying to ignore them
				if (!paren) ignore_parentheses = false;
			}
			else
			{
				// TODO: handle such keyword/operators with a system
				if (m_Identifier == "defined")
				{
					// since it's technically a unary operator (keyword and possible function, IMO),
					// we need to make sure we dont already have a value
					if (!got_val)
					{
						defined_operator = true;
						ignore_parentheses = true;
						m_DisableMacroExpansion = true;
					}
					else SendError(Error::expr_expected_operator, m_Token.Range());
				}
			}
			break;
		}

		// if we've got a value, we've probably got an operation to do!
		if (got_val) {
			// handle unary / 1-ary operations
			for (; !unary_operators.empty(); unary_operators.pop()) {
				bool b = ExpressUnary(unary_operators.top().first, val);
				if (!b) {
					SendError(Error::invalid_unary_operator, unary_operators.top().first, unary_operators.top().second);
				}
				ASSERT(b && "Non-unary operator in unary_operators!!!!");
			}

			// binary operators / 2-ary
			// AKA The ABC's...
			switch (op)
			{
				// Arithmetic
			default:
				result = val;
				break;
			case Operators::Type::add:
				result += val;
				break;
			case Operators::Type::sub:
				result -= val;
				break;
			case Operators::Type::mult:
				result *= val;
				break;
			case Operators::Type::div:
				result /= val;
				break;
			case Operators::Type::mod:
				result %= val;
				break;

				// Bitwise
			case Operators::Type::bit_and:
				result &= val;
				break;
			case Operators::Type::bit_or:
				result |= val;
				break;
			case Operators::Type::bit_xor:
				result ^= val;
				break;
			case Operators::Type::bit_shl:
				result <<= val;
				break;
			case Operators::Type::bit_shr:
				result >>= val;
				break;

				// Comparison
			case Operators::Type::eq:
				result = result == val;
				break;
			case Operators::Type::neq:
				result = result != val;
				break;
			case Operators::Type::leq:
				result = result <= val;
				break;
			case Operators::Type::geq:
				result = result >= val;
				break;
			case Operators::Type::lt:
				result = result < val;
				break;
			case Operators::Type::gt:
				result = result > val;
				break;
			}

			// evaluate logical operation
			if (log_op != Operators::max_operator && op != Operators::Type::and && op != Operators::Type::or)
			{
				if (log_op == Operators::Type::and)
					result = log_operand & (result != 0);
				else if (log_op == Operators::Type::or)
					result = log_operand | (result != 0);

				log_op = Operators::max_operator;
			}
		}
	}

	// evaluate logical operation
	if (log_op == Operators::Type::and)
		result = log_operand & (result != 0);
	else if (log_op == Operators::Type::or)
		result = log_operand | (result != 0);
			
	return result;
}
bool Preprocessor::Lexpect(SCRambl::TokenType type) {
	if (Lex() == Lexing::found_token) {
		if (m_Token == type) return true;
		switch (type) {
		case TokenType::Eol: SendError(Error::expected_eol);
			break;
		case TokenType::Identifier: SendError(Error::expected_identifier);
			break;
		case TokenType::String: SendError(Error::expected_string);
			break;
		case TokenType::Label: SendError(Error::expected_label);
			break;
		case TokenType::Number: SendError(Error::expected_number);
			break;
		case TokenType::Operator: SendError(Error::expected_operator);
			break;
		case TokenType::CloseParen: SendError(Error::expected_closing_paren);
			break;
		case TokenType::OpenParen: SendError(Error::expected_opening_paren);
			break;
		case TokenType::Separator: SendError(Error::expected_separator);
			break;
		}
	}
	return false;
}
Lexing::Result Preprocessor::Lex() {
	Lexing::Result result;

	// it is a crime to consider any of these a macro, under punishment of death by infinite recursion (a slow, painful way to go)
	std::unordered_set<std::string> identifiersThatAreNotMacros;

	while (true) {
		while (m_CodePos && m_CodePos->IsIgnorable())
			++m_CodePos;

		// If we're preprocessing a directive, directly return further directions
		if (m_State == found_directive) {
			m_OperatorScanner.Enable();			// enable preprocessor operators
			m_ParserOperatorScanner.Disable();	// disable parser (proper) operators

			if (m_CodePos->GetType() == Symbol::punctuator) {
				switch (char c = *m_CodePos)
				{
				case '(':
				case ')':
					m_Token(c == '(' ? TokenType::OpenParen : TokenType::CloseParen, m_CodePos, m_CodePos, m_CodePos + 1);
					m_CodePos = m_Token.End();
					return Lexing::Result::found_token;
				default:
					break;
				}
			}
			else if (m_CodePos->GetType() == Symbol::separator) {
				m_Token(TokenType::Separator, m_CodePos, m_CodePos, m_CodePos + 1);
				m_CodePos = m_Token.End();
				return Lexing::Result::found_token;
				//m_Token()
			}
		}
		else {
			m_OperatorScanner.Disable();		// disable preprocessor operators
			m_ParserOperatorScanner.Enable();	// enable parser (proper) operators
		}
				
		// try to lex something - catch and handle any thrown scanner errors
		try {
			result = m_CodePos ? m_Lexer.Scan(m_CodePos, m_Token) : Lexing::Result::found_nothing;
		}
		catch (const StringLiteralScanner::Error & err) {
			switch (err) {
				// unterminated string literal
			case StringLiteralScanner::Error::unterminated:
				// recovery method: skip to end of this line
				while (m_CodePos->GetType() != Symbol::eol)
					++m_CodePos;
				SendError(Error::unterminated_string_literal);
				break;
			}
			continue;
		}
		catch (const BlockCommentScanner::Error & err) {
			switch (err) {
				// unterminated block comment
			case BlockCommentScanner::Error::end_of_file_reached:
				SendError(Error::unterminated_block_comment);

				// recovery method: skip to end of this line (attempt to treat like a single line comment)
				while (m_CodePos->GetType() != Symbol::eol)
					++m_CodePos;
				break;
			}
			continue;
		}

		switch (result)
		{
		case Lexing::Result::still_scanning:
			if (m_CodePos->IsEOL())
			{
				AddToken<Tokens::Character::Info<Character>>(m_CodePos, Tokens::Type::Character, m_CodePos, Character::EOL);
				m_WasLastTokenEOL = true;
			}
			continue;
		case Lexing::Result::found_nothing:
			break;

		case Lexing::Result::found_token:
			// we found one, we found one!
			m_CodePos = m_Token.End();

			// only try to handle directives and comments if we're skipping source
			if (!GetSourceControl() && (m_Token != TokenType::Directive))
					continue;

			// tell brother
			m_Task.Event<event_found_token>(m_Token.Range());

			switch (m_Token)
			{
				/*\
					- Take care of these directly and continue until there's some real code...
				\*/
			case TokenType::Comment:
			case TokenType::BlockComment:
				// handle comments immediately - get rid o' that ol' waste o' space
				HandleComment();
				continue;

				/*\
					- These will be handled by the callee
				\*/
			case TokenType::Directive:
				// get the directive identifier and look up its ID
				m_Directive = GetDirective(m_Build.GetScript().GetCode().Select(m_Token.Inside(), m_Token.End()));

				// if the source is being skipped, wait until we have a related directive
				if (!GetSourceControl() && !DoesDirectiveIgnoreSourceControl(m_Directive))
					continue;
						
				// if the directive is invalid, send an error with the range of the identifier
				if (m_Directive == Directive::INVALID)
				{
					// ensure any report is at the beginning of the directive, not the end where we currently are
					//m_Information.SetScriptPos(m_Token.Begin());
					SendError(Error::invalid_directive, Scripts::Range(m_Token.Inside(), m_Token.End()));
					//m_Information.SetScriptPos(m_CodePos);

					// to recover, skip to the eol
					while (m_CodePos->GetType() != Symbol::eol) ++m_CodePos;
				}
				break;

			case TokenType::String:
				// save the string
				m_String = m_Build.GetScript().GetCode().Select(m_Token.Inside(), m_Token.End());
				m_String = m_String.substr(0, m_String.find_last_not_of('\0') + 1);
				break;

			case TokenType::Identifier:
				// save the identifier
				//m_Identifier = m_Script.GetCode().Select(m_Token.Begin(), m_Token.End());
				m_Identifier = m_Token.Range().Format();

				if (false)
				{
			case TokenType::Label:
				// save the label name
				m_Identifier = m_Token.Range().Format();
				if (m_CodePos->GetType() == Symbol::punctuator)
					++m_CodePos;
				}

				// in certain cases we may want the macros actual identifier
				if (!m_DisableMacroExpansion && !m_DisableMacroExpansionOnce)
				{
					// ensure this identifier isn't not a macro... double negative, yes, but there's a big difference
					if (identifiersThatAreNotMacros.find(m_Identifier) == identifiersThatAreNotMacros.end())
					{
						// check for macro
						if (auto * macro = m_Macros.Get(m_Identifier))
						{
							// NO! you CANNOT be a macro twice, don't be so stupid
							identifiersThatAreNotMacros.emplace(m_Identifier);

							// remove the identifier from code
							m_CodePos = m_Build.GetScript().GetCode().Erase(m_Token.Begin(), m_Token.End());

							// insert the macro code
							m_CodePos = m_Build.GetScript().GetCode().Insert(m_CodePos, macro->GetCode().Symbols());
							// continue parsing until we have a REAL token
							continue;
						}
					}
				}
				else m_DisableMacroExpansionOnce = false;
				break;
			}
			break;
		}

		break;
	}

	return result;
}
bool Preprocessor::LexNumber() {
	if (Lex() == Lexing::Result::found_token) {
		if (m_Token == TokenType::Number)
			return true;
	}
	return false;
}

void Preprocessor::PushSourceControl(bool b) {
	// if source is already deactivated, override b and deactivate again
	m_PreprocessorLogic.push(GetSourceControl() ? b : false);
}
void Preprocessor::PopSourceControl() {
	ASSERT(!m_PreprocessorLogic.empty() && "#endif when not in #if/#ifdef/#else?");
	m_PreprocessorLogic.pop();
}
void Preprocessor::InvertSourceControl() {
	ASSERT(!m_PreprocessorLogic.empty() && "Unmatched #else?");

	// don't invert if we're within an #if which is within an "#if FALSE"
	if (!GetSourceControl()) {
		PopSourceControl();
		PushSourceControl(GetSourceControl() ? true : false);
	}
	else m_PreprocessorLogic.top() = false;
}
bool Preprocessor::GetSourceControl() const {
	return m_PreprocessorLogic.empty() ? true : m_PreprocessorLogic.top();
}

bool Preprocessor::OpenDelimiter(Scripts::Position pos, Delimiter type) {
	auto range = Scripts::Range(pos, pos);
	auto token = m_Build.CreateToken<TokenDelimiter>(range, pos, range, type);
	m_Delimiters.emplace(token);
	return true;
}
bool Preprocessor::CloseDelimiter(Scripts::Position pos, Delimiter type) {
	auto token = m_Delimiters.top();
	auto tok = token->GetToken();
	// ensure the delimiters are for the same purpose, otherwise there's error
	if (Tokens::Delimiter::GetDelimiterType<Delimiter>(*tok) == type) {
		auto begin = Tokens::Delimiter::GetScriptRange(*tok).Begin();
		auto range = Scripts::Range(begin, pos);
		// replace the token with an updated Scripts::Range
		token->SetToken(new TokenDelimiter(begin, range, type));
		// mark the closing position
		m_Build.CreateToken<TokenDelimiter>(range, pos, range, type);
		m_Delimiters.pop();
		return true;
	}
	return false;
}
VecRef<Types::Type> Preprocessor::GetType(const std::string& name) {
	return m_Build.GetTypes().GetType(name).Ref();
}

// Printf-styled error reporting
template<typename... TArgs>
void Preprocessor::SendError(Error type, TArgs&&... args) {
	// send
	std::vector<std::string> params;
	m_Task.Event<error_event>(Basic::Error(m_Engine, type), params);
}
template<typename First, typename... Args>
void Preprocessor::SendError(Error type, First&& first, Args&&... args) {
	// storage for error parameters
	std::vector<std::string> params;
	// format the error parameters to the vector
	m_Engine.Format(params, first, args...);
	// send
	m_Task.Event<error_event>(Basic::Error(m_Engine, type), params);
}

// Scanners - BORING! ;)

// Scan nothing (as useless as it sounds)
bool WhitespaceScanner::Scan(Lexing::State& state, Scripts::Position& code) {
	if (state == Lexing::State::before) {
		if (code->GetType() == Symbol::whitespace) {
			// remove excess whitespace (?)
			auto next = code;
			++next;
			while (next && next->GetType() == Symbol::whitespace)
				next.Delete();
			return true;
		}
	}
	return false;
}
// Scan identifiers
bool IdentifierScanner::Scan(Lexing::State& state, Scripts::Position& pos) {
	switch (state) {
		// return true if the symbol can only be an identifier
	case Lexing::State::before:
		if (pos->GetType() == Symbol::identifier) {
			++pos;
			state = Lexing::State::inside;
			return true;
		}
		return false;

		// return true once we've read all of the identifier characters
	case Lexing::State::inside:
		while (pos && (pos->GetType() == Symbol::identifier || pos->GetType() == Symbol::number))
			++pos;
		state = Lexing::State::after;
		return true;

		// make sure that a separator followed the identifier chars - else throw a tantrum
	case Lexing::State::after:
		if (!pos || pos->IsSeparating()) return true;
		// throw()
		return false;
	}
	return false;
}
// Scan labels
bool LabelScanner::Scan(Lexing::State& state, Scripts::Position& pos) {
	switch (state) {
		// return true if the symbol can only be an identifier
	case Lexing::State::before:
		if (pos->GetType() == Symbol::identifier) {
			++pos;
			state = Lexing::State::inside;
			return true;
		}
		return false;

		// return true once we've read all of the identifier characters
	case Lexing::State::inside:
		while (pos && (pos->GetType() == Symbol::identifier || pos->GetType() == Symbol::number))
			++pos;
		state = Lexing::State::after;
		return true;

		// make sure that a : followed the identifier chars
	case Lexing::State::after:
		if (pos && pos->GetGrapheme() == Grapheme::colon) {
			// not if there's something important immediately after the colon...
			auto nextpos = pos + 1;
			if (!nextpos || nextpos->IsIgnorable() || nextpos->IsEOL()) {
				++pos;
				return true;
			}
		}
		return false;
	}
	return false;
}
// Scan directives
bool DirectiveScanner::Scan(Lexing::State& state, Scripts::Position& pos) {
	switch (state) {
		// check prefix
	case Lexing::State::before:
		if (pos->GetGrapheme() == Grapheme::hash)		// #
		{
			++pos;
			state = Lexing::State::inside;
			return true;
		}
		break;

		// check for first unfitting character
	case Lexing::State::inside:
		if (!pos || pos->GetType() != Symbol::identifier) return false;
		while (++pos && (pos->GetType() == Symbol::identifier || pos->GetType() == Symbol::number));
		state = Lexing::State::after;
		return true;

		// no suffix? no problem.
	case Lexing::State::after:
		if (pos->IsSeparating()) return true;
		//else throw "Invalid symbol in directive"
		return false;
	}
	return false;
}
// Scan string literals
bool StringLiteralScanner::Scan(Lexing::State& state, Scripts::Position& pos) {
	switch (state) {
	case Lexing::State::before:
		if (pos == '"') {
			++pos;
			state = Lexing::State::inside;
			return true;
		}
		return false;

	case Lexing::State::inside:
		while (pos) {
			// escaped? just skip it :)
			if (pos == '\\') {
				if (++pos) ++pos;
				continue;
			}
			else if (pos->GetType() == Symbol::eol) {
				throw(Error::unterminated);
			}
			else if (pos == '"') {
				*pos = '\0';
				++pos;
				state = Lexing::State::after;
				return true;
			}
			else ++pos;
		}
		return false;

	case Lexing::State::after:
		return true;
	}
	return false;
}
// Scan line comments
bool CommentScanner::Scan(Lexing::State& state, Scripts::Position& pos) {
	switch (state)
	{
		// return number of matched prefix chars
	case Lexing::State::before:
		if (pos == '/' && ++pos == '/') {
			++pos;
			state = Lexing::State::inside;
			return true;
		}
		return false;

		// run to the end of the line, quick!
	case Lexing::State::inside:
		while (pos && pos->GetType() != Symbol::eol) ++pos;
		state = Lexing::State::after;
		return true;

		// yeah, k
	case Lexing::State::after:
		return true;
	}
	return false;
}
// Scan block comments
bool BlockCommentScanner::Scan(Lexing::State& state, Scripts::Position& pos) {
	char last_char = '\0';
	switch (state) {
		// check for opening of block comment sequence
	case Lexing::State::before:
		// check for opening
		if (pos == '/' && ++pos == '*') {
			++pos;
			++depth;
			state = Lexing::State::inside;
			return true;
		}
		return false;

		// check for nested comments and closing comment
	case Lexing::State::inside:
		do {
			if (pos->GetType() == Symbol::punctuator) {
				if (pos == '/') {
					if (last_char == '*') {
						++pos;
						if (!--depth) {
							state = Lexing::State::after;
							return true;
						}
					}
				}
				else if (last_char == '/' && pos == '*') {
					++depth;
				}

				last_char = *pos;
			}
		} while (++pos);

		if (depth) throw(Error::end_of_file_reached);
		ASSERT(!depth);			// TODO: throw error "still in comment at end-of-file"
		return true;

	case Lexing::State::after:
		return true;
	}
	return false;
}

bool Task::IsRunning() const { return Preprocessor::IsRunning(); }
bool Task::IsTaskFinished() { return Preprocessor::IsFinished(); }
void Task::RunTask() { Preprocessor::Run(); }
void Task::ResetTask() { Preprocessor::Reset(); }
const Information& Task::Info() const { return m_Info; }
Task::Task(Engine& engine, Build* build) : TaskSystem::Task(build),
	Preprocessor(*this, engine, *build),
	m_Engine(engine), m_Info(GetInfo())
{ }