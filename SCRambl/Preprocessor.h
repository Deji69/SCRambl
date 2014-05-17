/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Tasks.h"
#include "Scripts.h"
#include "Lexer.h"

namespace SCRambl
{
	class WhitespaceScanner : public Lexer::Scanner
	{
	public:
		bool Scan(const Lexer::LexerState & state, const std::string & str, std::string::const_iterator & it) override
		{
			switch (state)
			{
			case Lexer::LexerState::before: return IsSpace(*it) != 0;
			case Lexer::LexerState::inside:
				while (IsSpace(*it)) ++it;
				return true;
			case Lexer::LexerState::after: return true;
			}
			return false;
		}
	};
	class IdentifierScanner : public Lexer::Scanner
	{
	public:
		bool Scan(const Lexer::LexerState & state, const std::string & str, std::string::const_iterator & it) override
		{
			switch (state)
			{
			case Lexer::LexerState::before:
				if (IsIdentifierStart(*it))
				{
					++it;
					return true;
				}
				break;
			case Lexer::LexerState::inside:
				return !IsIdentifier(*it++);
			case Lexer::LexerState::after:
				if (!IsSeparator(*it)) throw(*this);
				return true;
			}
			return false;
		}
	};
	class DirectiveScanner : public Lexer::Scanner
	{
	public:
		bool Scan(const Lexer::LexerState & state, const std::string & str, std::string::const_iterator & it) override
		{
			switch (state)
			{
				// check prefix
			case Lexer::LexerState::before:
				if (*it == '#')
				{
					++it;
					return true;
				}
				break;

				// check for first unfitting character
			case Lexer::LexerState::inside:
				if (IsIdentifier(*it))
				{
					++it;
					return true;
				}
				break;

				// no suffix? no problem.
			case Lexer::LexerState::after:
				return true;
			}
			return false;
		}
	};

	class CommentScanner : public Lexer::Scanner
	{
	public:
		bool Scan(const Lexer::LexerState & state, const std::string & str, std::string::const_iterator & it) override
		{
			switch (state)
			{
				// return number of matched prefix chars
			case Lexer::LexerState::before:
				if (it[0] == '/' && it[1] == '/')
				{
					it += 2;
					return true;
				}
				break;

				// keep going until EOL
			case Lexer::LexerState::inside:
				if (*it == '\n' || *it == '\r')
					return true;
				break;

				// yeah, k
			case Lexer::LexerState::after:
				return true;
			}
			return false;
		}
	};

	class BlockCommentScanner : public Lexer::Scanner
	{
		int				depth = 0;
		char			last_char = 0;

	public:
		bool Scan(const Lexer::LexerState & state, const std::string & str, std::string::const_iterator & it) override
		{
			switch (state)
			{
				// check for opening of block comment sequence
			case Lexer::LexerState::before:
				if (*it != '/' || it[1] != '*')
					break;
				it += 2;
				return true;

				// check for nested comments
			case Lexer::LexerState::inside:
				if (*it == '*')
				{
					if (last_char == '/')
						++depth;
					else if (it[1] == '/')
					{
						if (!depth)
						{
							it += 2;
							return true;
						}
						--depth;
					}
				}
				break;

			case Lexer::LexerState::after:
				return true;
			}

			last_char = *it;
			return false;
		}
	};

	class Preprocessor : virtual private Task
	{
		enum Token
		{
			token_none,
			token_directive,
			token_comment,
			token_block_comment,
			token_invalid,
			token_max = token_invalid
		};

		Engine						&	m_Engine;
		Lexer::Lexer<Token>				m_Lexer;
		DirectiveScanner				m_DirectiveScanner;
		CommentScanner					m_CommentScanner;
		BlockCommentScanner				m_BlockCommentScanner;
		WhitespaceScanner				m_WhitespaceScanner;
		std::string					*	m_Code;
		std::string::iterator			m_CodeIterator;

	public:
		enum State {
			init,
			begin_line, end_of_line,
			before_directive, during_directive, after_directive,
			before_comment, during_comment, after_comment,
			finished,
			bad_state,
			max_state = bad_state,		// deprecated
		};
		/*enum Error {
			error_
		};*/

		Preprocessor(Engine &, Script &);

		inline bool IsFinished()				{ return m_State == finished; }
		void Run();
		void Reset();

	private:
		State					m_State;
		Script				&	m_Script;
		CodeList::iterator		m_ScriptLine;
		bool					m_bScriptIsLoaded;		// if so, we only need to add-in any #include's

		void RunningState();
		inline bool NextLine()					{ return ++m_ScriptLine != m_Script.Code().end(); }
		inline int GetLineNumber() const		{ return *m_ScriptLine; }
		inline std::string & GetLineCode()		{ return *m_ScriptLine; }
	};

	class PreprocessorTask : public Preprocessor, virtual public Task
	{
	public:
		PreprocessorTask(Engine & engine, Script & script) : Preprocessor(engine, script)
		{
		}

	protected:
		bool IsTaskFinished() final override	{ return Preprocessor::IsFinished(); }
		void RunTask() final override			{ Preprocessor::Run(); }
		void ResetTask() final override			{ Preprocessor::Reset(); }
	};


	/*class Preprocessor : public PreprocessorState
	{
	public:
		Task & RunTask() override;
		Task & AdvanceState(Task::State & state) override;
	};*/
}