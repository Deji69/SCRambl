#include "stdafx.h"
#include "Preprocessor.h"
#include "Lexer.h"

namespace SCRambl
{
	Preprocessor::Preprocessor(Engine & engine, Script & script) : m_State(init), m_Engine(engine), m_Script(script), m_Lexer(), m_CodePos(script.GetCode())
	{
		Reset();
		m_Lexer.AddTokenScanner<WhitespaceScanner>(token_none, m_WhitespaceScanner);
		m_Lexer.AddTokenScanner<BlockCommentScanner>(token_block_comment, m_BlockCommentScanner);
		m_Lexer.AddTokenScanner<CommentScanner>(token_comment, m_CommentScanner);
		m_Lexer.AddTokenScanner<DirectiveScanner>(token_directive, m_DirectiveScanner);
		//m_Lexer.AddTokenScanner<IdentifierScanner>(token_identifier, m_IdentifierScanner);

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

		/*if (m_CodeIterator == std::end(m_Code->Symbols()))
		{
			if (m_State == inside_comment)
				HandleComment();			// delete everything on line from the comment and continue it on the next line

			if (!NextLine())
			{
				m_State = finished;
				return;
			}

			m_State = lexing;
			return;
		}*/
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

		m_State = lexing;
	}

	void Preprocessor::HandleComment()
	{
		// handle it with care by deleting the shit out of it
		m_CodePos = m_Script.GetCode().Erase(m_Token.Begin(), m_Token.End());

		m_State = lexing;
	}

	void Preprocessor::LexerPhase()
	{
		while (m_CodePos && m_CodePos->GetType() == Symbol::eol) ++m_CodePos;

		if (!m_CodePos)
		{
			m_State = finished;
			return;
		}

		switch (m_Lexer.Scan(m_CodePos, m_Token))
		{
		case Lexer::ScanResult::still_scanning:
			return;

		case Lexer::ScanResult::found_token:
			// we found one, we found one!
			switch (m_Token)
			{
			case token_comment:
			case token_block_comment:
				m_State = found_comment;
				return;
			case token_directive:
				m_State = found_directive;
				return;
			}
			return;

		case Lexer::ScanResult::found_nothing:
			std::cerr << "found_nothing\n";
			return;
		}
	}
}