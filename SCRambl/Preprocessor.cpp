#include "stdafx.h"
#include "Preprocessor.h"
#include "Lexer.h"

namespace SCRambl
{
	Preprocessor::Preprocessor(Engine & engine, Script & script) : m_State(init), m_Engine(engine), m_Script(script), m_Lexer(), m_CodePos(script.GetCode())
	{
		Reset();
		m_Lexer.AddTokenScanner(token_none, m_WhitespaceScanner);
		m_Lexer.AddTokenScanner(token_block_comment, m_BlockCommentScanner);
		m_Lexer.AddTokenScanner(token_comment, m_CommentScanner);
		m_Lexer.AddTokenScanner(token_directive, m_DirectiveScanner);
		m_Lexer.AddTokenScanner(token_string, m_StringLiteralScanner);
		m_Lexer.AddTokenScanner(token_identifier, m_IdentifierScanner);

		m_Directives["include"] = directive_include;
		m_Directives["define"] = directive_define;
		m_Directives["ifdef"] = directive_ifdef;
		m_Directives["elif"] = directive_elif;
		m_Directives["else"] = directive_else;
		m_Directives["endif"] = directive_endif;
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

			m_State = during_directive;
			return;

		case directive_ifdef:
			if (Lex() == Lexer::Result::found_token && m_Token == token_identifier)
			{
				PushSourceControl(m_Macros.Get(m_Identifier) != nullptr);
			}
			//else
			return;

		case directive_else:
			InvertSourceControl();
			return;

		case directive_endif:
			PopSourceControl();
			return;

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
				}
				//else throw("failed to include file...")
			}
			// else throw "expected string"
			return;

		default:
			BREAK();
		}

		m_State = lexing;
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
		// we only want #'s, comments and identifiers for macro replacement...
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

	Lexer::Result Preprocessor::Lex()
	{
		Lexer::Result result;
		
		while (true)
		{
			while (m_CodePos && m_CodePos->IsIgnorable()) ++m_CodePos;

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
					break;
				case token_string:
					// save the string
					m_String = m_Script.GetCode().Select(m_Token.Inside(), m_Token.End());
					break;
				case token_identifier:
					// save the identifier
					m_Identifier = m_Script.GetCode().Select(m_Token.Begin(), m_Token.End());
					break;
				}
				break;
			}
			
			break;
		}

		return result;
	}
}