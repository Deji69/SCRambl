#include "stdafx.h"
#include "Directives.h"
#include "Preprocessor.h"
#include "Lexer.h"

namespace SCRambl
{
	namespace Preprocessor
	{
		Preprocessor::Preprocessor(Engine & engine, Script & script):
			m_State(init),
			m_Engine(engine),
			m_Script(script),
			m_Lexer(),
			m_CodePos(script.GetCode()),
			m_OperatorScanner(m_Operators)
		{
			Reset();
			m_Lexer.AddTokenScanner(Token::None, m_WhitespaceScanner);
			m_Lexer.AddTokenScanner(Token::BlockComment, m_BlockCommentScanner);
			m_Lexer.AddTokenScanner(Token::Comment, m_CommentScanner);
			m_Lexer.AddTokenScanner(Token::Directive, m_DirectiveScanner);
			m_Lexer.AddTokenScanner(Token::String, m_StringLiteralScanner);
			m_Lexer.AddTokenScanner(Token::Identifier, m_IdentifierScanner);
			m_Lexer.AddTokenScanner(Token::Number, m_NumericScanner);
			m_Lexer.AddTokenScanner(Token::Operator, m_OperatorScanner);

			m_Directives["define"] = directive_define;
			m_Directives["elif"] = directive_elif;
			m_Directives["else"] = directive_else;
			m_Directives["endif"] = directive_endif;
			m_Directives["if"] = directive_if;
			m_Directives["ifdef"] = directive_ifdef;
			m_Directives["include"] = directive_include;

			// arithmetic
			m_Operators.AddOperator({ { '+' } }, Operator::add);
			m_Operators.AddOperator({ { '-' } }, Operator::sub);
			m_Operators.AddOperator({ { '*' } }, Operator::mult);
			m_Operators.AddOperator({ { '/' } }, Operator::div);
			m_Operators.AddOperator({ { '%' } }, Operator::mod);

			// bitwise
			m_Operators.AddOperator({ { '&' } }, Operator::bit_and);
			m_Operators.AddOperator({ { '|' } }, Operator::bit_or);
			m_Operators.AddOperator({ { '^' } }, Operator::bit_xor);
			m_Operators.AddOperator({ { '<', '<' } }, Operator::bit_shl);
			m_Operators.AddOperator({ { '>', '>' } }, Operator::bit_shr);

			// comparison
			//m_Operators.AddOperator({ { '=' } }, Operator::eq);
			m_Operators.AddOperator({ { '>' } }, Operator::gt);
			m_Operators.AddOperator({ { '<' } }, Operator::lt);
			m_Operators.AddOperator({ { '=', '=' } }, Operator::eq);
			m_Operators.AddOperator({ { '>', '=' } }, Operator::geq);
			m_Operators.AddOperator({ { '<', '=' } }, Operator::leq);

			// logical
			m_Operators.AddOperator({ { '!' } }, Operator::not);
			m_Operators.AddOperator({ { '&', '&' } }, Operator::and);
			m_Operators.AddOperator({ { '|', '|' } }, Operator::or);
			m_Operators.AddOperator({ { '?' } }, Operator::cond);
			m_Operators.AddOperator({ { ':' } }, Operator::condel);
		}

		void Preprocessor::Reset()
		{
			switch (m_State)
			{
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

		void Preprocessor::Run()
		{
			try
			{
				switch (m_State)
				{
				case init:
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

		void Preprocessor::RunningState()
		{
			auto old_state = m_State;

			switch (m_State)
			{
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
			}
		}

		void Preprocessor::HandleDirective()
		{
			switch (m_Directive)
			{
			case directive_define:
				if (Lex() == Lexer::Result::found_token && m_Token == Token::Identifier)
				{
					Macro::Name name = m_Identifier;

					// skip to the good bit
					while (m_CodePos && m_CodePos->IsIgnorable()) ++m_CodePos;

					// gather up thy symbols
					if (m_CodePos && m_CodePos->GetType() != Symbol::eol)
					{
						CodeLine::vector code;

						auto start_pos = m_CodePos;
						while (m_CodePos && m_CodePos->GetType() != Symbol::eol)
							++m_CodePos;

						m_Macros.Define(name, m_Script.GetCode().Copy(start_pos, m_CodePos, code));
					}
					else m_Macros.Define(name);
				}
				break;

			case directive_if:
				PushSourceControl(ProcessExpression() != 0);
				break;

			case directive_ifdef:
				if (Lex() == Lexer::Result::found_token && m_Token == Token::Identifier)
				{
					PushSourceControl(m_Macros.Get(m_Identifier) != nullptr);
				}
				break;

			case directive_else:
				InvertSourceControl();
				break;

			case directive_endif:
				PopSourceControl();
				break;

			case directive_include:
				if (Lex() == Lexer::Result::found_token && (m_Token == Token::String || m_Token == Token::Identifier))
				{
					if (m_Token == Token::Identifier)
					{
						if (auto pMacro = m_Macros.Get(m_String))
						{
						}
						//
					}
					if (m_Script.Include(m_CodePos, m_String))
					{
						m_State = lexing;
						return;
					}
					//else throw("failed to include file...")
				}
				// else throw "expected string"
				break;

			default:
				BREAK();
			}

			m_State = lexing;
			return;
		}

		void Preprocessor::HandleComment()
		{
			// handle it with care by deleting the shit out of it
			m_CodePos = m_Script.GetCode().Erase(m_Token.Begin(), m_Token.End());

			// that felt good... add a single space in its place as an extra sign of indignity...
			m_CodePos.Insert(' ');

			m_State = lexing;
		}

		void Preprocessor::LexerPhase()
		{
			// here we only want #'s, comments and identifiers for macro replacement...
			if (m_State == lexing)
			{
				while (m_CodePos && *m_CodePos != '#' && *m_CodePos != '/' && m_CodePos->GetType() != Symbol::identifier)
					++m_CodePos;
			}

			// ya, we're done here...
			if (!m_CodePos)
			{
				m_State = finished;
				return;
			}

			// Hey, Lex!
			switch (Lex())
			{
			case Lexer::Result::found_token:
				// handle state-changing token types
				switch (m_Token)
				{
				case Token::Directive:
					m_State = found_directive;
					return;
				}

				m_State = lexing;
				return;

			case Lexer::Result::found_nothing:
				++m_CodePos;
				m_State = lexing;
				return;
			}
		}

		bool Preprocessor::ExpressUnary(Operator::Type op, int & val)
		{
			switch (op)
			{
			default: return false;

			case Operator::not:
				val = !val;
				break;
			case Operator::bit_not:
				val = ~val;
				break;
			case Operator::sub:
				val = -val;
				break;
			case Operator::add:
				val = +val;			// lol
				break;
			}
			return true;
		}

		int Preprocessor::ProcessExpression(bool paren)
		{
			int result = 0;
			int val = 0;
			bool got_val = false;
			Operator::Type last_op = Operator::max_operator;
			Operator::Type op = Operator::max_operator;

			std::stack<Operator::Type>	unary_operators;

			while (true)
			{
				// Lex
				if (Lex() == Lexer::Result::found_nothing) break;

				// Do stuff
				switch (m_Token)
				{
				case Token::OpenParen:
					val = ProcessExpression(true);
					got_val = true;
					break;

				case Token::CloseParen:
					//if (!paren) throw("Unmatched closing parenthesis")
					return result;

				case Token::Number:
					ASSERT(!got_val && "Add error handling");
					ASSERT(!m_NumericScanner.Is<float>() && "Add error handling");

					val = m_NumericScanner.Get<int>();

					// handle unary / 1-ary operations
					for (; !unary_operators.empty(); unary_operators.pop())
						ASSERT(ExpressUnary(unary_operators.top(), val) && "Non-unary operator in unary_operators!!!!");
					
					got_val = true;
					break;

				case Token::Operator:
					//if (op == Operator::max_operator) throw;
					last_op = op;
					switch (op = m_OperatorScanner.GetOperator())
					{
						// Arithmetic
					case Operator::add:			// +
						// if not, do the binary operation instead
						if (!got_val) {
							// this is unary, correct things
							unary_operators.push(Operator::add);
							op = last_op;
						}
						else got_val = false;
						break;

					case Operator::sub:			// -
						// if not, do the binary operation instead
						if (!got_val) {
							// this is unary, correct things
							unary_operators.push(Operator::sub);
							op = last_op;
						}
						else got_val = false;
						break;

					case Operator::mult:		// *
					case Operator::div:			// /
					case Operator::mod:			// %
						got_val = false;
						break;

						// Bitwise
					case Operator::bit_and:
					case Operator::bit_or:
					case Operator::bit_xor:
					case Operator::bit_shl:
					case Operator::bit_shr:
						got_val = false;
						break;

					case Operator::bit_not:
						// if not, wait, this IS not!
						if (!got_val) {
							// this is unary, correct things
							unary_operators.push(Operator::bit_not);
							op = last_op;
						}
						break;

						// Comparison
					case Operator::gt:			// >
					case Operator::lt:			// <
					case Operator::geq:			// >=
					case Operator::leq:			// <=
					case Operator::eq:			// ==
						got_val = false;
						break;

						// Logical
					case Operator::not:			// !
						// if not, wait, THIS IS NOT TOOO!!!
						if (!got_val) {
							// this is unary, correct things
							unary_operators.push(Operator::not);
							op = last_op;
						}
						break;

					case Operator::cond:		// ?
					case Operator::condel:		// :
					default:
						got_val = false;
						break;
					}

					ASSERT ((!got_val || op != Operator::max_operator) && "Premature error in testing phase (unary used AFTER a value)");
					break;
				}

				// binary operators / 2-ary
				if (got_val) {
					// The ABC's...
					switch (op)
					{
						// Arithmetic
					default:
						result = val;
						break;
					case Operator::add:
						result += val;
						break;
					case Operator::sub:
						result -= val;
						break;
					case Operator::mult:
						result *= val;
						break;
					case Operator::div:
						result /= val;
						break;
					case Operator::mod:
						result %= val;
						break;

						// Bitwise
					case Operator::bit_and:
						result &= val;
						break;
					case Operator::bit_or:
						result |= val;
						break;
					case Operator::bit_xor:
						result ^= val;
						break;
					case Operator::bit_shl:
						result <<= val;
						break;
					case Operator::bit_shr:
						result >>= val;
						break;

						// Comparison
					case Operator::eq:
						result = result == val;
						break;
					case Operator::neq:
						result = result != val;
						break;
					case Operator::leq:
						result = result <= val;
						break;
					case Operator::geq:
						result = result >= val;
						break;
					case Operator::lt:
						result = result < val;
						break;
					case Operator::gt:
						result = result > val;
						break;
					}
				}
			}
			return result;
		}

		Lexer::Result Preprocessor::Lex()
		{
			Lexer::Result result;

			while (true)
			{
				while (m_CodePos && m_CodePos->IsIgnorable()) ++m_CodePos;

				// If we're preprocessing a directive, directly return further directions
				if (m_State == found_directive && m_CodePos->GetType() == Symbol::punctuator)
				{
					switch (char c = *m_CodePos)
					{
					case '(':
					case ')':
						m_Token(c == '(' ? Token::OpenParen : Token::CloseParen, m_CodePos, m_CodePos, m_CodePos + 1);
						m_CodePos = m_Token.End();
						return Lexer::Result::found_token;
					default:
						break;
					}
				}

				switch (result = m_CodePos ? m_Lexer.Scan(m_CodePos, m_Token) : Lexer::Result::found_nothing)
				{
				case Lexer::Result::still_scanning: continue;
				case Lexer::Result::found_nothing: break;

				case Lexer::Result::found_token:
					// we found one, we found one!
					m_CodePos = m_Token.End();

					switch (m_Token)
					{
						/*\
						 | Take care of these directly and continue until there's some real code...
						 \*/
					case Token::Comment:
					case Token::BlockComment:
						// handle comments immediately - get rid o' that ol' waste o' space
						HandleComment();
						continue;

						/*\
						 | These will be handled by the callee
						 \*/
					case Token::Directive:
						// get the directive identifier and look up its ID
						m_Directive = GetDirective(m_Script.GetCode().Select(m_Token.Inside(), m_Token.End()));
						ASSERT(m_Directive != directive_invalid);
						break;

					case Token::String:
						// save the string
						m_String = m_Script.GetCode().Select(m_Token.Inside(), m_Token.End());
						break;

					case Token::Identifier:
						// save the identifier
						m_Identifier = m_Script.GetCode().Select(m_Token.Begin(), m_Token.End());

						// check for macro
						if (auto * macro = m_Macros.Get(m_Identifier))
						{
							// remove the identifier from code
							m_CodePos = m_Script.GetCode().Erase(m_Token.Begin(), m_Token.End());

							// insert the macro code
							m_CodePos = m_Script.GetCode().Insert(m_CodePos, macro->GetCode().Symbols());

							// continue parsing until we have a REAL token
							continue;
						}
						break;

					}
					break;
				}

				break;
			}

			return result;
		}

		bool Preprocessor::LexNumber()
		{
			if (Lex() == Lexer::Result::found_token)
			{
				if (m_Token == Token::Number)
					return true;
			}
			return false;
		}
	}
}