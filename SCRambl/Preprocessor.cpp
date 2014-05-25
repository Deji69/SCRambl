#include "stdafx.h"
#include "Preprocessor.h"
#include "Lexer.h"

namespace SCRambl
{
	Preprocessor::Preprocessor(Engine & engine, Script & script) : m_State(init), m_Engine(engine), m_Script(script), m_Lexer(), m_Code(nullptr)
	{
		Reset();
		m_Lexer.AddTokenScanner<WhitespaceScanner>(token_none, m_WhitespaceScanner);
		m_Lexer.AddTokenScanner<BlockCommentScanner>(token_block_comment, m_BlockCommentScanner);
		m_Lexer.AddTokenScanner<CommentScanner>(token_comment, m_CommentScanner);
		m_Lexer.AddTokenScanner<DirectiveScanner>(token_directive, m_DirectiveScanner);
		m_Lexer.AddTokenScanner<IdentifierScanner>(token_identifier, m_IdentifierScanner);

		m_Directives["include"] = directive_include;
		m_Directives["define"] = directive_define;
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

		m_State = lexing;
		m_ScriptLine = m_Script.Code().begin();
		m_Code = &GetLineCode();
		m_CodeIterator = m_Code->begin();
	}

	void Preprocessor::Run()
	{
		try
		{
			switch (m_State)
			{
			case init:
				throw("Preprocessor was left un-initialised");
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
		case idle:
		case lexing:
		default:
			LexerPhase();
			break;
		case found_comment:
			HandleComment();
			break;
		case found_directive:
			HandleDirective();
			break;
		}

		if (m_State == idle)
		{
			if (m_CodeIterator == m_Code->end())
			{
				if (m_State == inside_comment)
					HandleComment();			// delete everything on line from the comment and continue it on the next line

				if (!NextLine())
				{
					m_State = finished;
					return;
				}

				m_State = idle;
				return;
			}
		}
	}

	void Preprocessor::HandleDirective()
	{
		switch (m_Directive)
		{
		case directive_define:
		case during_directive:

			LexerPhase();

			m_State = during_directive;
			return;

		default:
			BREAK();
		}

		m_State = idle;
	}

	void Preprocessor::HandleComment()
	{
		switch (m_Lexer.GetTokenType())
		{
		case token_comment:
			// just remove the whole line
			*m_Code = "";
			m_CodeIterator = m_Code->end();
			break;
		case token_block_comment:
			// replace it with a single whitespace character (for the parsing stage) and skip it ourselves
			m_Code->replace(m_Lexer.GetTokenStart(), m_Lexer.GetTokenEnd(), " ");
			m_CodeIterator = std::next(m_Lexer.GetTokenStart());
			break;
		default:
			BREAK();
		}

		m_State = idle;
	}

	void Preprocessor::LexerPhase()
	{
		switch (m_Lexer.GetState())
		{
		case Lexer::LexerState::before:
			// we're in the preprocessor, so just ignore anything alien
			while (!m_Lexer.Scan(*m_Code, m_CodeIterator) && ++m_CodeIterator != m_Code->end());
			// possible scanner match! next state...
			m_State = lexing;
			return;

		case Lexer::LexerState::inside:
			if (m_Lexer.Scan(m_CodeIterator)) {
				// end of the token? next state...

				m_State = lexing;
				return;
			}
			else {
				// the code is still being scanned

				switch (m_Lexer.Tokenize())
				{
				case token_block_comment:
					// if a block comment is being scanned, make sure we know so we can remove commented code from this line if it ends before the comment
					m_State = inside_comment;
					return;
				default:
					m_State = lexing;
					return;
				}
			}
			break;

		case Lexer::LexerState::after:
			// see if the token is valid
			if (m_Lexer.Scan(m_CodeIterator))
			{
				// grab the token, do stuff
				auto token = m_Lexer.Tokenize();

				switch (token)
				{
				case token_whitespace:
					// trim excessive whitespace down to one character and skip it
					m_Code->replace(m_Lexer.GetTokenStart(), m_Lexer.GetTokenEnd(), " ");
					m_CodeIterator = m_Lexer.GetTokenStart() + 1;
					break;

				case token_directive:
					// get the directive and enter the directive handling state
					m_Directive = GetDirective(token);
					m_State = found_directive;
					return;

				case token_comment:
				case token_block_comment:
					// enter comment handling state
					m_State = found_comment;
					return;
				}

				// nothing special
				m_State = idle;
				return;
			}
			else
			{
				// scanner invalidated. if any following scanner validates the code, the state will now be 'inside',
				// otherwise no scanner validated the code and the process may begin again with new code
				m_State = m_Lexer.GetState() == Lexer::LexerState::inside ? lexing : idle;
				return;
			}
			break;
		}
	}
}