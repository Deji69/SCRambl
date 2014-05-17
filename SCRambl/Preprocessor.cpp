#include "stdafx.h"
#include "Preprocessor.h"
#include "Lexer.h"

namespace SCRambl
{
	Preprocessor::Preprocessor(Engine & engine, Script & script) : m_State(init), m_Engine(engine), m_Script(script), m_Lexer(), m_Code(nullptr)
	{
		Reset();
		m_Lexer.AddTokenScanner<WhitespaceScanner>(token_none, m_WhitespaceScanner);
		m_Lexer.AddTokenScanner<DirectiveScanner>(token_directive, m_DirectiveScanner);
		m_Lexer.AddTokenScanner<CommentScanner>(token_comment, m_CommentScanner);
		m_Lexer.AddTokenScanner<BlockCommentScanner>(token_block_comment, m_BlockCommentScanner);
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

		m_State = begin_line;
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
		auto & code = *m_Code;
		std::string::const_iterator & it = m_CodeIterator;

		if (m_State == end_of_line)
		{
			if (!NextLine())
			{
				m_State = finished;
				return;
			}
			
			m_State = begin_line;
			return;
		}
		
		if (it == code.end())
		{
			m_State = end_of_line;
			return;
		}

		// Triple lex rated
		switch (m_Lexer.GetState())
		{
		case Lexer::LexerState::before:
			if (m_Lexer.Scan(code, it)) {
				// possible scanner match? next state...
			}
			else {
				// the start of the current code was not detected by any scanner
				// no need to throw an error in the preprocessor, however
			}
			break;
		case Lexer::LexerState::inside:
			if (m_Lexer.Scan(it)) {
				// end of the token? next state...
			}
			else {
				// the code is still being scanned
			}
			break;
		case Lexer::LexerState::after:
			if (m_Lexer.Scan(it))
			{
				// grab the current token
				auto token = m_Lexer.Tokenize();
				switch (token)
				{
				case token_directive:
					m_State = after_directive;
					return;
				}
			}
			else
			{
				// scanner invalidated. if any following scanner validates the code, the state will now be 'inside',
				// otherwise no scanner validated the code and the process will begin again expecting new code
			}
			break;
		}
		
		/*switch (m_State)
		{
		case begin_line:
		{
			// Beginning of line, skip spaces and check for directive as the first token

			/*code = ltrim(code);
			if (code[0] == '#')
			{
				m_State = before_directive;
				break;
			}*\/
			break;
		}
		case before_directive:
			
			m_State = during_directive;
			break;
		case during_directive:
			break;
		}*/
	}
}