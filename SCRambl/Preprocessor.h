/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <stack>
#include "Tasks.h"
#include "Engine.h"
#include "Scripts.h"
#include "Lexer.h"
#include "Macros.h"
#include "Identifiers.h"
#include "Operators.h"
#include "Numbers.h"
#include "Tokens.h"

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
					if (pos->GetGrapheme() == Grapheme::hash)		// #
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
					if (pos->IsSeparating()) return true;
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

		class Task;

		/*class Event
		{
		public:
			enum Type
			{
				directive_not_found,
				unknown,
			};
			enum class Severity
			{
				notice, warning, error
			};

		private:
			Type			m_Type;
			Severity		m_Severity;

		public:
			inline Type			GetType() const				{ return m_Type; }
			inline Severity		GetSeverity() const			{ return m_Severity; }

			inline operator Type() const					{ return GetType(); }
		};*/

		class Preprocessor
		{
		private:
			enum Directive
			{
				directive_invalid,
				directive_include,
				directive_define,
				directive_ifdef,
				directive_ifndef,
				directive_if,
				directive_elif,
				directive_else,
				directive_endif,
				directive_undef,
			};

			using DirectiveMap = std::unordered_map<std::string, Directive>;

			//Engine									&	m_Engine;
			Task									&	m_Task;

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

			Lexer::Lexer<Token::Type>					m_Lexer;
			Lexer::Token<Token::Type>					m_Token;
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
				found_directive,
				found_comment,
				found_token,
				finished,
				bad_state,
				max_state = bad_state,
			};

			Preprocessor(Task &, Script &);

			inline bool IsFinished() const			{ return m_State == finished; }
			void Run();
			void Reset();

		private:
			State					m_State = init;
			Script				&	m_Script;
			Script::Position		m_CodePos;
			bool					m_bScriptIsLoaded;		// if so, we only need to add-in any #include's
			bool					m_DisableMacroExpansion = false;
			bool					m_DisableMacroExpansionOnce = false;
			std::stack<bool>		m_PreprocessorLogic;

			void PushSourceControl(bool b) {
				m_PreprocessorLogic.push(b);
			}
			void PopSourceControl() {
				ASSERT(!m_PreprocessorLogic.empty() && "#endif when not in #if/#ifdef/#else?");
				m_PreprocessorLogic.pop();
			}
			inline void InvertSourceControl() {
				ASSERT(!m_PreprocessorLogic.empty() && "Unmatched #else?");
				m_PreprocessorLogic.top() = !m_PreprocessorLogic.top();
			}
			inline bool GetSourceControl() const {
				//ASSERT(!m_PreprocessorLogic.empty() && "not in #if/#ifdef/#else?");
				return m_PreprocessorLogic.empty() ? true : m_PreprocessorLogic.top();
			}

			void RunningState();
			void LexerPhase();

			void HandleToken();
			void HandleDirective();
			void HandleComment();

			// Lex main code
			Lexer::Result Lex();
			// Lex around for a number
			bool LexNumber();

			// Handle expressions
			int ProcessExpression(bool paren = false);
			// Perform unary operation on passed value - returns false if no change could be made as the operator was unsupported
			static bool ExpressUnary(Operator::Type op, int & val);

			// Get current line number
			inline long GetLineNumber() const			{ return m_CodePos.GetLine(); }

			// Get code of the current line
			inline CodeLine & GetLineCode()				{ return m_CodePos.GetLine().GetCode(); }

			// Returns directive_invalid if it didnt exist
			inline Directive GetDirective(const std::string & str) const
			{
				DirectiveMap::const_iterator it;
				return it = m_Directives.find(str), it != m_Directives.end() ? (*it).second : directive_invalid;
			}
		};

		// yes, you do
		class IBaseInformation
		{

		};

		template<typename... Args>
		class Information : IBaseInformation
		{
			std::tuple<Args...>		m_Info;

		public:
			inline operator const std::tuple<Args...>&() const			{ return m_Info; }
		};

		class Error
		{
		public:
			enum Type {
				invalid_directive,
			};

		private:
			Type								m_Type;
			std::shared_ptr<IBaseInformation>	m_Info;
			
		public:
			Error(Type type, std::shared_ptr<IBaseInformation> info) : m_Type(type), m_Info(info)
			{
			}

			template<typename... Args>
			std::shared_ptr<Information<Args...>> const&	Info()		{ return static_cast<Information<Args...>>(m_Info); }

			inline operator Type() const		{ return m_Type; }
		};

		enum class Event
		{
			Begin,
			Warning,
			Error,
		};

		/*\
		 * The Preprocessor and Task become one
		\*/
		class Task : public TaskSystem::Task<Event>, private Preprocessor
		{
			friend Preprocessor;
			Engine			&	m_Engine;

			inline Engine	&	GetEngine()			{ return m_Engine; }

			template<typename... Args>
			inline bool operator()(Event id, Args&&... args)	{ return CallEventHandler(id, std::forward<Args...>(args)...); }

		public:
			Task(Engine & engine, Script & script):
				Preprocessor(*this, script),
				m_Engine(engine)
			{
				/*AddEvent<Event::Begin>();
				AddEvent<Event::Warning>();
				AddEvent<Event::Error>();*/
			}

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