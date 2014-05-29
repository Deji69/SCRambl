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
#include "Macros.h"
#include "Identifiers.h"

namespace SCRambl
{
	class WhitespaceScanner : public Lexer::Scanner
	{
	public:
		bool Scan(Lexer::State & state, CodeLine & line, CodeLine::iterator & it) override
		{
			if (state == Lexer::State::before)
			{
				if (it->GetType() == Symbol::whitespace)
				{
					// remove excess whitespace
					while (true)
					{
						auto nex = std::next(it);
						if (nex == line.Symbols().end() || nex->GetType() != Symbol::whitespace)
							break;
						line.Symbols().erase(nex);
					}
					return true;
				}
			}
			return false;
		}
	};
	/*class IdentifierScanner : public Lexer::Scanner
	{
	public:
		bool Scan(const Lexer::State & state, const std::string & str, std::string::const_iterator & it) override
		{
			switch (state)
			{
				// return true if IsIdentifierStart
			case Lexer::State::before:
				if (IsIdentifierStart(*it))
				{
					++it;
					return true;
				}
				break;
				// return true once we've read all the identifier characters
			case Lexer::State::inside:
				return !IsIdentifier(*it++);
				// make sure that a separator followed the identifier chars - else throw a tantrum
			case Lexer::State::after:
				if (!IsSeparator(*it)) throw(*this);
				return true;
			}
			return false;
		}
	};*/
	class DirectiveScanner : public Lexer::Scanner
	{
	public:
		bool Scan(Lexer::State & state, CodeLine & line, CodeLine::iterator & it) override
		{
			switch (state)
			{
				// check prefix
			case Lexer::State::before:
				if (*it == '#')
				{
					state = Lexer::State::inside;
					++it;
					return true;
				}
				break;

				// check for first unfitting character
			case Lexer::State::inside:
				if (it->GetType() == Symbol::identifier || it->GetType() == Symbol::number)
					state = Lexer::State::after;
				++it;
				return true;

				// no suffix? no problem.
			case Lexer::State::after:
				if (it->GetType() == Symbol::whitespace || it->GetType() == Symbol::separator)
				{
				}
				return true;
			}
			return false;
		}
	};

	class CommentScanner : public Lexer::Scanner
	{
	public:
		bool Scan(Lexer::State & state, CodeLine & str, CodeLine::iterator & it) override
		{
			switch (state)
			{
				// return number of matched prefix chars
			case Lexer::State::before:
				if (*it++ == '/' && it != std::end(str.Symbols()) && *it++ == '/')
					return true;
				break;

				// dont bother scanning the rest of the line - the preprocessor will only be deleting it - just the '//' will do
			case Lexer::State::inside:
				return true;

				// yeah, k
			case Lexer::State::after:
				return true;
			}
			return false;
		}
	};

	class BlockCommentScanner : public Lexer::Scanner
	{
		int				depth = 0;

	public:
		bool Scan(Lexer::State & state, CodeLine & str, CodeLine::iterator & it) override
		{
			switch (state)
			{
				// check for opening of block comment sequence
			case Lexer::State::before:
				if (depth)
				{
					// we're already inside a block comment
					state = Lexer::State::inside;
					return true;
				}

				// check for opening
				if (*it == '/')
				{
					++it;
					if (it != std::end(str.Symbols()) && *it == '*')
					{
						++it;
						++depth;
						state = Lexer::State::inside;
						return true;
					}
				}
				break;

				// check for nested comments and closing comment
			case Lexer::State::inside:
				if (it != std::end(str.Symbols()))
				{
					if (*it == Symbol::punctuator)
					{
						switch (*it)
						{
						case '/':
							++it;
							if (it != std::end(str.Symbols()) && *it == '*')
							{
								++it;
								++depth;
							}
							break;

						case '*':
							++it;
							if (it != std::end(str.Symbols()) && *it == '/')
							{
								++it;
								if (!--depth)
								{
									state = Lexer::State::after;
									return true;
								}
							}
							break;

						default:
							++it;
							break;
						}
					}
					else ++it;
				}
				return true;

			case Lexer::State::after:
				return true;
			}

			return false;
		}
	};

	class Preprocessor
	{
		enum Token
		{
			token_none,
			token_whitespace,					// we'll be trimming whitespace, tabs, comments, etc. down to one single white space character for easier parsing
			token_eol,
			token_identifier,
			token_directive,
			token_comment,
			token_block_comment,
			token_invalid,
			token_max = token_invalid
		};
		enum Directive
		{
			directive_include,
			directive_define,
			directive_invalid,
		};

		using DirectiveMap = std::unordered_map<std::string, Directive>;

		Engine									&	m_Engine;

		//IdentifierScanner							m_IdentifierScanner;
		DirectiveScanner							m_DirectiveScanner;
		CommentScanner								m_CommentScanner;
		BlockCommentScanner							m_BlockCommentScanner;
		WhitespaceScanner							m_WhitespaceScanner;

		Lexer::Lexer<Token>							m_Lexer;
		CodeLine								*	m_Code;
		CodeLine::iterator							m_CodeIterator;
		std::string::iterator						m_SavedCodePos;
		DirectiveMap								m_Directives;
		Directive									m_Directive;
		MacroMap									m_Macros;

	public:
		enum State {
			init,
			//begin_line, during_line, end_of_line,
			idle, lexing,
			found_directive, during_directive,
			found_comment, inside_comment,
			finished,
			bad_state,
			max_state = bad_state,
		};

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
		void LexerPhase();

		void HandleDirective();
		void HandleComment();

		inline int GetLineNumber() const			{ return *m_ScriptLine; }
		inline CodeLine & GetLineCode()				{ return *m_ScriptLine; }

		// Returns true if there was another line
		bool NextLine()
		{
			// if we managed to get a new line, get the new code
			if (++m_ScriptLine != std::end(m_Script.Code()))
			{
				m_Code = &GetLineCode();
				m_CodeIterator = std::begin(m_Code->Symbols());
				return true;
			}
			return false;
		}

		// Returns directive_invalid if it didnt exist
		inline Directive GetDirective(const std::string & str) const
		{
			DirectiveMap::const_iterator it;
			return it = m_Directives.find(str), it != m_Directives.end() ? (*it).second : directive_invalid;
		}
	};

	class PreprocessorTask : public Task, public Preprocessor
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