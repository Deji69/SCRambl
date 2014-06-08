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
			m_Lexer.AddTokenScanner(token_none, m_WhitespaceScanner);
			m_Lexer.AddTokenScanner(token_block_comment, m_BlockCommentScanner);
			m_Lexer.AddTokenScanner(token_comment, m_CommentScanner);
			m_Lexer.AddTokenScanner(token_directive, m_DirectiveScanner);
			m_Lexer.AddTokenScanner(token_string, m_StringLiteralScanner);
			m_Lexer.AddTokenScanner(token_identifier, m_IdentifierScanner);
			m_Lexer.AddTokenScanner(token_number, m_NumericScanner);
			m_Lexer.AddTokenScanner(token_operator, m_OperatorScanner);

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
				if (Lex() == Lexer::Result::found_token && m_Token == token_identifier)
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
				if (Lex() == Lexer::Result::found_token && m_Token == token_identifier)
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
				if (Lex() == Lexer::Result::found_token && (m_Token == token_string || m_Token == token_identifier))
				{
					if (m_Token == token_identifier)
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
				case token_directive:
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

		int Preprocessor::ProcessExpression(bool paren)
		{
			int result = 0;
			int val = 0;
			bool got_val = false;
			bool negate_val = false;
			Operator::Type op = Operator::max_operator;

			while (true)
			{
				// Lex
				if (Lex() == Lexer::Result::found_nothing) break;

				// Do stuff
				switch (m_Token)
				{
				case token_open_paren:
					val = ProcessExpression(true);
					break;

				case token_close_paren:
					//if (!paren) throw("Unmatched closing parenthesis")
					return val;

				case token_number:
					// if(m_NumericScanner.IsFloat()) throw("blah blah");
					if (got_val) {
						// error
					}
					val = negate_val ? -m_NumericScanner.GetInt() : m_NumericScanner.GetInt();
					got_val = true;
					negate_val = false;
					break;

				case token_operator:
					//if (op == Operator::max_operator) throw;
					switch (op = m_OperatorScanner.GetOperator())
					{
						// Arithmetic
					case Operator::add:			// +
						if (!got_val)
						{
							if (negate_val) negate_val = false;
							// else throw("Unexpected operator")
							op = Operator::max_operator;
						}
						else {
							got_val = false;
						}
						// else throw("Expected operand");
						break;
					case Operator::sub:			// -
						// negate?
						if (!got_val)
						{
							if (!negate_val) negate_val = true;
							//else error
							op = Operator::max_operator;
						}
						else {
							got_val = false;
						}
						break;
					case Operator::mult:
						break;
					case Operator::div:
						break;
					case Operator::mod:
						break;

						// Bitwise
					case Operator::bit_and:
						break;
					case Operator::bit_or:
						break;
					case Operator::bit_xor:
						break;
					case Operator::bit_shl:
						break;
					case Operator::bit_shr:
						break;

						// Comparison
					case Operator::gt:			// >
						break;
					case Operator::lt:			// <
						break;
					case Operator::geq:			// >=
						break;
					case Operator::leq:			// <=
						break;
					case Operator::eq:			// ==
						break;

						// Logical
					case Operator::cond:		// ?
						break;
					case Operator::condel:		// :
						break;
					}
					break;
				}

				if (got_val) {
					switch (op)
					{
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
					}
				}
			}
			return val;
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
						m_Token(c == '(' ? token_open_paren : token_close_paren, m_CodePos, m_CodePos, m_CodePos + 1);
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
					case token_comment:
					case token_block_comment:
						// handle comments immediately - get rid o' that ol' waste o' space
						HandleComment();
						continue;

						/*\
						 | These will be handled by the callee
						 \*/
					case token_directive:
						// get the directive identifier and look up its ID
						m_Directive = GetDirective(m_Script.GetCode().Select(m_Token.Inside(), m_Token.End()));
						ASSERT(m_Directive != directive_invalid);
						break;

					case token_string:
						// save the string
						m_String = m_Script.GetCode().Select(m_Token.Inside(), m_Token.End());
						break;

					case token_identifier:
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
				if (m_Token == token_number)
					return true;
			}
			return false;
		}
	}
}