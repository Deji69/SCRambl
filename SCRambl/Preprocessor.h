/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <stack>
#include "Tasks.h"
#include "Scripts.h"
#include "Lexer.h"
#include "Macros.h"
#include "Identifiers.h"
#include "Operators.h"
#include "Numbers.h"

namespace SCRambl
{
	namespace Preprocessor
	{
		class WhitespaceScanner : public Lexer::Scanner
		{
		public:
			bool Scan(Lexer::State & state, Script::Position & code) override
			{
				if (state == Lexer::State::before)
				{
					if (code->GetType() == Symbol::whitespace)
					{
						// remove excess whitespace
						auto next = code;
						++next;
						while (next && next->GetType() == Symbol::whitespace)
							next.Delete();
						return true;
					}
				}
				return false;
			}
		};

		class IdentifierScanner : public Lexer::Scanner
		{
		public:
			bool Scan(Lexer::State & state, Script::Position & pos) override
			{
				switch (state)
				{
					// return true if the symbol can only be an identifier
				case Lexer::State::before:
					if (pos->GetType() == Symbol::identifier)
					{
						++pos;
						state = Lexer::State::inside;
						return true;
					}
					return false;

					// return true once we've read all of the identifier characters
				case Lexer::State::inside:
					while (pos && (pos->GetType() == Symbol::identifier || pos->GetType() == Symbol::number))
						++pos;
					state = Lexer::State::after;
					return true;

					// make sure that a separator followed the identifier chars - else throw a tantrum
				case Lexer::State::after:
					if (!pos || pos->IsSeparating())
					{
						return true;
					}
					// throw()
					return false;
				}
				return false;
			}
		};

		class DirectiveScanner : public Lexer::Scanner
		{
		public:
			bool Scan(Lexer::State & state, Script::Position & pos) override
			{
				switch (state)
				{
					// check prefix
				case Lexer::State::before:
					if (pos == '#')
					{
						++pos;
						state = Lexer::State::inside;
						return true;
					}
					break;

					// check for first unfitting character
				case Lexer::State::inside:
					if (!pos || pos->GetType() != Symbol::identifier) return false;
					while (++pos && pos->GetType() == Symbol::identifier || pos->GetType() == Symbol::number);
					state = Lexer::State::after;
					return true;

					// no suffix? no problem.
				case Lexer::State::after:
					if (pos->IsSeparating())
					{
						return true;
					}
					//else throw "Invalid symbol in directive"
					return false;
				}
				return false;
			}
		};

		class StringLiteralScanner : public Lexer::Scanner
		{
		public:
			bool Scan(Lexer::State & state, Script::Position & pos) override
			{
				switch (state)
				{
				case Lexer::State::before:
					if (pos == '"')
					{
						++pos;
						state = Lexer::State::inside;
						return true;
					}
					return false;

				case Lexer::State::inside:
					while (pos)
					{
						// escaped? just skip it :)
						if (pos == '\\')
						{
							if (++pos) ++pos;
							continue;
						}
						else if (pos->GetType() == Symbol::eol)
						{
							// throw("unterminated string");
							break;
						}
						else if (pos == '"')
						{
							*pos = '\0';
							++pos;
							state = Lexer::State::after;
							return true;
						}
						else ++pos;
					}
					return false;

				case Lexer::State::after:
					return true;
				}
				return false;
			}
		};

		class CommentScanner : public Lexer::Scanner
		{
		public:
			bool Scan(Lexer::State & state, Script::Position & pos) override
			{
				switch (state)
				{
					// return number of matched prefix chars
				case Lexer::State::before:
					if (pos == '/' && ++pos == '/')
					{
						++pos;
						state = Lexer::State::inside;
						return true;
					}
					return false;

					// run to the end of the line, quick!
				case Lexer::State::inside:
					while (pos && pos->GetType() != Symbol::eol) ++pos;
					state = Lexer::State::after;
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
			bool Scan(Lexer::State & state, Script::Position & pos) override
			{
				char last_char = '\0';
				switch (state)
				{
					// check for opening of block comment sequence
				case Lexer::State::before:
					// check for opening
					if (pos == '/' && ++pos == '*')
					{
						++pos;
						++depth;
						state = Lexer::State::inside;
						return true;
					}
					return false;

					// check for nested comments and closing comment
				case Lexer::State::inside:
					do {
						if (pos->GetType() == Symbol::punctuator)
						{
							if (pos == '/')
							{
								if (last_char == '*')
								{
									++pos;
									if (!--depth)
									{
										state = Lexer::State::after;
										return true;
									}
								}
							}
							else if (last_char == '/' && pos == '*')
							{
								++depth;
							}

							last_char = *pos;
						}
					} while (++pos);
					ASSERT(!depth);			// TODO: throw error "still in comment at end-of-file"
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
				token_open_paren,
				token_close_paren,
				token_comment,
				token_block_comment,
				token_number,
				token_operator,
				token_string,
				token_invalid,
				token_max = token_invalid
			};
			enum Directive
			{
				directive_invalid,
				directive_include,
				directive_define,
				directive_ifdef,
				directive_if,
				directive_elif,
				directive_else,
				directive_endif
			};

			using DirectiveMap = std::unordered_map<std::string, Directive>;

			Engine									&	m_Engine;

			//IdentifierScanner							m_IdentifierScanner;
			BlockCommentScanner							m_BlockCommentScanner;
			CommentScanner								m_CommentScanner;
			DirectiveScanner							m_DirectiveScanner;
			IdentifierScanner							m_IdentifierScanner;
			NumericScanner								m_NumericScanner;
			StringLiteralScanner						m_StringLiteralScanner;
			WhitespaceScanner							m_WhitespaceScanner;

			Operator::Table	< Operator::Type, Operator::max_operator >		m_Operators;
			Operator::Scanner < Operator::Type, Operator::max_operator >	m_OperatorScanner;

			Lexer::Lexer<Token>							m_Lexer;
			Lexer::Token<Token>							m_Token;
			DirectiveMap								m_Directives;
			Directive									m_Directive = directive_invalid;
			std::string									m_String;
			std::string									m_Identifier;
			MacroMap									m_Macros;

		public:
			enum State {
				init,
				//begin_line, during_line, end_of_line,
				lexing,
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
			State					m_State = init;
			Script				&	m_Script;
			Script::Position		m_CodePos;
			bool					m_bScriptIsLoaded;		// if so, we only need to add-in any #include's
			std::stack<bool>		m_PreprocessorLogic;

			void PushSourceControl(bool b) {
				m_PreprocessorLogic.push(b);
			}
			void PopSourceControl() {
				ASSERT(!m_PreprocessorLogic.empty());
				m_PreprocessorLogic.pop();
			}
			void InvertSourceControl() {
				ASSERT(!m_PreprocessorLogic.empty());
				m_PreprocessorLogic.top() = !m_PreprocessorLogic.top();
			}
			bool GetSourceControl() const {
				ASSERT(!m_PreprocessorLogic.empty()); // if this activates, you popped too much!
				return m_PreprocessorLogic.top();
			}

			void RunningState();
			void LexerPhase();

			void HandleDirective();
			void HandleComment();

			// Lex main code
			Lexer::Result Lex();
			// Lex around for a number
			bool LexNumber();

			// Handle expressions
			int ProcessExpression(bool paren = false);

			inline long GetLineNumber() const			{ return m_CodePos.GetLine(); }
			inline CodeLine & GetLineCode()				{ return m_CodePos.GetLine().GetCode(); }

			// Returns directive_invalid if it didnt exist
			inline Directive GetDirective(const std::string & str) const
			{
				DirectiveMap::const_iterator it;
				return it = m_Directives.find(str), it != m_Directives.end() ? (*it).second : directive_invalid;
			}
		};
		
		class Token
		{

		};

		class Task : public TaskBase, public Preprocessor
		{
		public:
			Task(Engine & engine, Script & script) : Preprocessor(engine, script)
			{ }

		protected:
			bool IsTaskFinished() final override	{ return Preprocessor::IsFinished(); }
			void RunTask() final override			{ Preprocessor::Run(); }
			void ResetTask() final override			{ Preprocessor::Reset(); }
		};
	}

	/*class PreprocessorTask : public Task, public Preprocessor::Preprocessor
	{
	public:
		PreprocessorTask(Engine & engine, Script & script) : Preprocessor(engine, script)
		{ }

	protected:
		bool IsTaskFinished() final override	{ return Preprocessor::IsFinished(); }
		void RunTask() final override			{ Preprocessor::Run(); }
		void ResetTask() final override			{ Preprocessor::Reset(); }
	};*/
}