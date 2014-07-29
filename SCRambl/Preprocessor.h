/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <stack>
#include <typeinfo>
#include <typeindex>
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
		/*\
		 * WhitespaceScanner - Lexer::Scanner for nothingness (useless?)
		\*/
		class WhitespaceScanner : public Lexer::Scanner
		{
		public:
			bool Scan(Lexer::State & state, Script::Position & code) override;
		};

		/*\
		 * IdentifierScanner - Lexer::Scanner for identifiers
		\*/
		class IdentifierScanner : public Lexer::Scanner
		{
		public:
			bool Scan(Lexer::State & state, Script::Position & pos) override;
		};

		/*\
		 * DirectiveScanner - Lexer::Scanner for directives
		\*/
		class DirectiveScanner : public Lexer::Scanner
		{
		public:
			bool Scan(Lexer::State & state, Script::Position & pos) override;
		};

		/*\
		 * StringLiteralScanner - Lexer::Scanner for string literals
		\*/
		class StringLiteralScanner : public Lexer::Scanner
		{
		public:
			enum class Error {
				unterminated
			};

			bool Scan(Lexer::State & state, Script::Position & pos) override;
		};

		/*\
		 * CommentScanner - Lexer::Scanner for line comments
		\*/
		class CommentScanner : public Lexer::Scanner
		{
		public:
			bool Scan(Lexer::State & state, Script::Position & pos) override;
		};

		/*\
		 * BlockCommentScanner - Lexer::Scanner for block comments
		\*/
		class BlockCommentScanner : public Lexer::Scanner
		{
			int				depth = 0;

		public:
			enum class Error {
				end_of_file_reached,
			};

			bool Scan(Lexer::State & state, Script::Position & pos) override;
		};

		class Task;

		/*\
		 * Preprocessor::Information - Externally accessible info about the preprocessors current state
		\*/
		class Information
		{
			friend class Preprocessor;
			Script::Position		&	m_ScriptPosition;

			void SetScriptPos(Script::Position & pos)				{ m_ScriptPosition = pos; }

		public:
			Information(Script::Position & pos):
				m_ScriptPosition(pos)
			{}

			inline const Script::Position &	GetScriptPos() const	{ return m_ScriptPosition; }
		};

		/*\
		 * Preprocessor::Error - Errors that can happen while preprocessing
		\*/
		class Error
		{
		public:
			enum ID {
				// involuntary errors (errors that should be impossible)
				internal_invalid_unary_operator		= 500,

				// normal errors
				invalid_directive						= 1000,
				unterminated_block_comment,				// 1001
				unterminated_string_literal,			// 1002
				expr_expected_operator,					// 1003
				expr_unexpected_float,					// 1004
				expr_unmatched_closing_parenthesis,		// 1005
				expr_invalid_operator,					// 1006
				expected_identifier,					// 1007
				expected_expression,					// 1008
				invalid_unary_operator,					// 1009
				invalid_unary_operator_use,				// 1010
			};

			Error(ID id) : m_ID(id)
			{ }
			inline operator ID() const			{ return m_ID; }

		private:
			ID			m_ID;
		};

		/*\
		 * Preprocessor::Directive - Preprocessor directive stuff
		\*/
		class Directive
		{
		public:
			enum Type {
				INVALID,
				INCLUDE,
				DEFINE,
				IFDEF,
				IFNDEF,
				IF,
				ELIF,
				ELSE,
				ENDIF,
				UNDEF,
			};

		private:
			Type		m_Type;

		public:
			Directive() = default;
			Directive(Type type) : m_Type(type)
			{ }

			inline operator Type() const		{ return m_Type; }

			static inline std::string Formatter(Directive type) {
				std::string name = "";
				switch (type) {
				default:
				case INVALID: break;
				case INCLUDE: name = "include";
				case DEFINE: name = "define";
				case IFDEF: name = "ifdef";
				case IFNDEF: name = "ifndef";
				case IF: name = "if";
				case ELIF: name = "elif";
				case ELSE: name = "else";
				case ENDIF: name = "endif";
				}
				return !name.empty() ? ("#" + name) : "(invalid)";
			}
		};

		/*\
		 - Preprocessor::Preprocessor - Main Preprocessor task routine
		\*/
		class Preprocessor
		{
			friend class Information;

		private:
			using LexerToken = Lexer::Token < Token::Type >;
			using LexerMachine = Lexer::Lexer < Token::Type >;
			using DirectiveMap = std::unordered_map < std::string, Directive >;
			using OperatorTable = Operator::Table < Operator::Type, Operator::max_operator >;
			using OperatorScanner = Operator::Scanner < Operator::Type, Operator::max_operator >;

			Engine					&	m_Engine;
			Task					&	m_Task;
			Information					m_Information;

			BlockCommentScanner			m_BlockCommentScanner;
			CommentScanner				m_CommentScanner;
			DirectiveScanner			m_DirectiveScanner;
			IdentifierScanner			m_IdentifierScanner;
			NumericScanner				m_NumericScanner;
			StringLiteralScanner		m_StringLiteralScanner;
			WhitespaceScanner			m_WhitespaceScanner;

			OperatorTable				m_Operators;
			OperatorScanner				m_OperatorScanner;

			LexerMachine				m_Lexer;
			LexerToken					m_Token;
			DirectiveMap				m_Directives;
			Directive					m_Directive = Directive::INVALID;
			std::string					m_String;
			std::string					m_Identifier;
			MacroMap					m_Macros;

		public:
			enum State {
				init,
				lexing,
				found_directive,
				found_comment,
				found_token,
				finished,
				bad_state,
				max_state = bad_state,
			};

			Preprocessor(Task &, Engine &, Script &);

			inline bool IsFinished() const			{ return m_State == finished; }
			void Run();
			void Reset();

		protected:
			const Information & GetInfo() const		{ return m_Information; }

		private:
			State								m_State = init;
			Script							&	m_Script;
			Script::Position					m_CodePos;
			bool								m_bScriptIsLoaded;						// if so, we only need to add-in any #include's
			bool								m_DisableMacroExpansion = false;
			bool								m_DisableMacroExpansionOnce = false;
			std::stack<bool>					m_PreprocessorLogic;
			
			// Send an error event
			void SendError(Error);
			template<typename First, typename... Args> void SendError(Error, First&&, Args&&...);

			// Enter conditional source compilation
			void PushSourceControl(bool b) {
				m_PreprocessorLogic.push(b);
			}
			// Return from conditional source compilation
			void PopSourceControl() {
				ASSERT(!m_PreprocessorLogic.empty() && "#endif when not in #if/#ifdef/#else?");
				m_PreprocessorLogic.pop();
			}
			// Invert conditional source compilation state
			inline void InvertSourceControl() {
				ASSERT(!m_PreprocessorLogic.empty() && "Unmatched #else?");
				m_PreprocessorLogic.top() = !m_PreprocessorLogic.top();
			}
			// Get conditional source compilation state
			inline bool GetSourceControl() const {
				return m_PreprocessorLogic.empty() ? true : m_PreprocessorLogic.top();
			}

			// Runs while running
			void RunningState();
			// Runs while lexing
			void LexerPhase();

			// Build preprocessing token from lex token
			void HandleToken();
			// Process preprocessor directive
			void HandleDirective();
			// Strips comments
			void HandleComment();

			// Gather information
			Information & BuildInformation(Information & info) const;

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
			inline Directive GetDirective(const std::string & str) const {
				DirectiveMap::const_iterator it;
				return it = m_Directives.find(str), it != m_Directives.end() ? (*it).second : Directive::INVALID;
			}
		};

		/*\
		 * Preprocessor::Event - Interesting stuff that the Preprocessor does
		\*/
		enum class Event
		{
			Begin,
			Warning,
			Error,
			FoundToken,
		};

		/*\
		 * Preprocessor::Task - The Preprocessor and Task become one
		\*/
		class Task : public TaskSystem::Task<Event>, private Preprocessor
		{
			friend Preprocessor;
			Engine				&	m_Engine;
			const Information	&	m_Info;

			inline Engine	&	GetEngine()				{ return m_Engine; }

			inline bool operator()(Event id)			{ return CallEventHandler(id); }

			template<typename... Args>
			inline bool operator()(Event id, Args&&... args)	{ return CallEventHandler(id, std::forward<Args>(args)...); }

		public:
			Task(Engine & engine, Script & script):
				Preprocessor(*this, engine, script),
				m_Engine(engine), m_Info(GetInfo())
			{
			}

			const Information & Info() const		{ return m_Info; }

		protected:
			bool IsTaskFinished() final override	{ return Preprocessor::IsFinished(); }
			void RunTask() final override			{ Preprocessor::Run(); }
			void ResetTask() final override			{ Preprocessor::Reset(); }
		};
	}
}