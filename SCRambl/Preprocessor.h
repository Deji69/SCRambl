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
#include <unordered_set>
#include "Tasks.h"
#include "Engine.h"
#include "Scripts.h"
#include "Lexer.h"
#include "Macros.h"
#include "Identifiers.h"
#include "Operators.h"
#include "Numbers.h"
#include "Labels.h"
#include "TokenInfo.h"
#include "Tokens.h"

namespace SCRambl
{
	namespace Preprocessor
	{
		/*\ WhitespaceScanner - Lexer::Scanner for nothingness (useless?) \*/
		class WhitespaceScanner : public Lexer::Scanner
		{
		public:
			bool Scan(Lexer::State& state, Scripts::Position& code) override;
		};

		/*\ IdentifierScanner - Lexer::Scanner for identifiers \*/
		class IdentifierScanner : public Lexer::Scanner
		{
		public:
			bool Scan(Lexer::State& state, Scripts::Position& pos) override;
		};

		/*\ LabelScanner - Lexer::Scanner for labels \*/
		class LabelScanner : public Lexer::Scanner
		{
		public:
			bool Scan(Lexer::State& state, Scripts::Position& pos) override;
		};

		/*\ DirectiveScanner - Lexer::Scanner for directives \*/
		class DirectiveScanner : public Lexer::Scanner
		{
		public:
			bool Scan(Lexer::State& state, Scripts::Position& pos) override;
		};

		/*\ StringLiteralScanner - Lexer::Scanner for string literals \*/
		class StringLiteralScanner : public Lexer::Scanner
		{
		public:
			enum class Error {
				unterminated
			};

			bool Scan(Lexer::State& state, Scripts::Position& pos) override;
		};

		/*\ CommentScanner - Lexer::Scanner for line comments \*/
		class CommentScanner : public Lexer::Scanner
		{
		public:
			bool Scan(Lexer::State& state, Scripts::Position& pos) override;
		};

		/*\ BlockCommentScanner - Lexer::Scanner for block comments \*/
		class BlockCommentScanner : public Lexer::Scanner
		{
			int depth = 0;

		public:
			enum class Error {
				end_of_file_reached,
			};

			bool Scan(Lexer::State& state, Scripts::Position& pos) override;
		};

		class Task;

		template<typename T, Numbers::Type TType>
		class TokenNumber : public Tokens::Number::Info<T> {
			using Parent = Tokens::Number::Info<T>;
			T m_Val;

		public:
			TokenNumber(Scripts::Range rg, T val) : Parent(Tokens::Type::Number, rg, TType, &m_Val),
				m_Val(val)
			{ }
		};

		/*\ Preprocessor::Information - Externally accessible info about the preprocessors current state \*/
		class Information {
			friend class Preprocessor;
			Scripts::Position& m_ScriptPosition;

			void SetScriptPos(Scripts::Position& pos) { m_ScriptPosition = pos; }

		public:
			Information(Scripts::Position& pos) : m_ScriptPosition(pos)
			{ }

			inline const Scripts::Position& GetScriptPos() const { return m_ScriptPosition; }
		};

		/*\ Preprocessor::Error - Errors that can happen while preprocessing \*/
		class Error {
		public:
			enum ID {
				// involuntary errors (errors that should be impossible)
				internal_invalid_unary_operator		= 500,
				internal_unable_to_allocate_token	= 501,

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
				dir_expected_file_name,					// 1011
				dir_expected_command_id,				// 1012
				dir_expected_identifier,				// 1013
				expected_eol,							// 1014
				expected_string,						// 1015
				expected_label,							// 1016
				expected_number,						// 1017
				expected_operator,						// 1018
				expected_closing_paren,					// 1019
				expected_opening_paren,					// 1020
				expected_separator,						// 1021
				expr_unmatched_closing_delimiter,		// 1022

				// fatal errors
				fatal_begin								= 4000,
				include_failed							= 4000,
				fatal_end,
			};

			Error(ID id) : m_ID(id)
			{ }
			inline operator ID() const { return m_ID; }

		private:
			ID m_ID;
		};

		/*\ Preprocessor::Directive - Preprocessor directive stuff \*/
		class Directive {
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

				REGISTER_VAR,
				REGISTER_COMMAND,
			};

			Directive() = default;
			Directive(Type type) : m_Type(type)
			{ }

			inline operator Type() const { return m_Type; }

			static inline std::string Formatter(Directive type) {
				std::string name = "";
				switch (type) {
				default:
				case INVALID: break;
				case REGISTER_VAR: name = "register_var";
					break;
				case REGISTER_COMMAND: name = "register_command";
					break;
				case INCLUDE: name = "include";
					break;
				case DEFINE: name = "define";
					break;
				case IFDEF: name = "ifdef";
					break;
				case IFNDEF: name = "ifndef";
					break;
				case IF: name = "if";
					break;
				case ELIF: name = "elif";
					break;
				case ELSE: name = "else";
					break;
				case ENDIF: name = "endif";
					break;
				}
				return !name.empty() ? ("#" + name) : "(invalid)";
			}

		private:
			Type m_Type;
		};

		/*\ Preprocessor::Character - Preprocessor character stuff \*/
		class Character {
		public:
			enum Type { EOL, Colonnector };

			Character() = default;
			Character(Type type) : m_Type(type) { }
			inline operator Type() const { return m_Type; }

			static inline std::string Formatter(Character type) {
				std::string rep;
				switch (type) {
				case EOL: rep = "[EOL]";
					break;
				case Colonnector: rep = ":";
					break;
				}
				return !rep.empty() ? rep : "[UNK]";
			}

		private:
			Type m_Type;
		};

		/*\ Preprocessor::Delimiter - Preprocessor delimiter stuff \*/
		class Delimiter
		{
		public:
			enum Type {
				None, Scope, Subscript, Cast
			};

			Delimiter() = default;
			Delimiter(Type type) : m_Type(type) { }

			inline operator Type() const { return m_Type; }
			
		private:
			Type m_Type = None;
		};

		/*\ Preprocessor::Preprocessor - Main Preprocessor task routine \*/
		class Preprocessor
		{
			friend class Information;

		private:
			using LexerToken = Lexer::Token<TokenType>;
			using LexerMachine = Lexer::Lexer<TokenType>;
			using DirectiveMap = std::unordered_map<std::string, Directive>;
			using OperatorTable = Operators::Table<Operators::Type>;
			using OperatorScanner = Operators::Scanner<Operators::Type>;
			template<typename... T>
			using TToken = TokenInfo<Tokens::Type, T...>;

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

			Preprocessor(Task&, Engine&, Build&);

			inline bool IsRunning() const { return m_State != finished; }
			inline bool IsFinished() const { return m_State == finished; }
			void Run();
			void Reset();

		protected:
			const Information & GetInfo() const { return m_Information; }

		private:
			Engine&	m_Engine;
			Task& m_Task;
			Information m_Information;

			BlockCommentScanner m_BlockCommentScanner;
			CommentScanner m_CommentScanner;
			DirectiveScanner m_DirectiveScanner;
			IdentifierScanner m_IdentifierScanner;
			Numbers::Scanner m_NumericScanner;
			LabelScanner m_LabelScanner;
			StringLiteralScanner m_StringLiteralScanner;
			WhitespaceScanner m_WhitespaceScanner;

			OperatorTable m_Operators;
			OperatorScanner m_OperatorScanner;

			Operators::Table<Operators::OperatorRef>& m_ParserOperators;
			Operators::Scanner<Operators::OperatorRef> m_ParserOperatorScanner;

			LexerMachine m_Lexer;
			LexerToken m_Token;
			DirectiveMap m_Directives;
			Directive m_Directive = Directive::INVALID;
			std::string m_String;					// last scanned string
			std::string	m_Identifier;				// last scanned identifier
			MacroMap m_Macros;

			std::stack<Scripts::Token> m_Delimiters;

			//
			State m_State = init;
			Build& m_Build;
			Scripts::Tokens& m_Tokens;
			Scripts::Position m_CodePos;
			Types::Types& m_Types;
			Commands& m_Commands;
			bool m_bScriptIsLoaded;						// if so, we only need to add-in any #include's
			bool m_DisableMacroExpansion = false;
			bool m_DisableMacroExpansionOnce = false;
			bool m_WasLastTokenEOL = false;
			unsigned long m_FalseLogicDepth = 0;					// for #if..#endif's ocurring within "#if FALSE"
			std::stack<bool> m_PreprocessorLogic;
			std::unordered_set<std::string> m_IdentifiersThatAreNotMacros;
			
			// Send an error event
			void SendError(Error);
			template<typename First, typename... Args> void SendError(Error, First&&, Args&&...);

			// Enter conditional source compilation
			void PushSourceControl(bool b) {
				// if source is already deactivated, override b and deactivate again
				m_PreprocessorLogic.push(GetSourceControl() ? b : false);
			}
			// Return from conditional source compilation
			void PopSourceControl() {
				ASSERT(!m_PreprocessorLogic.empty() && "#endif when not in #if/#ifdef/#else?");
				m_PreprocessorLogic.pop();
			}
			// Invert conditional source compilation state
			void InvertSourceControl() {
				ASSERT(!m_PreprocessorLogic.empty() && "Unmatched #else?");
				
				// don't invert if we're within an #if which is within an "#if FALSE"
				if (!GetSourceControl()) {
					PopSourceControl();
					PushSourceControl(GetSourceControl() ? true : false);
				}
				else m_PreprocessorLogic.top() = false;
			}
			// Get conditional source compilation state
			inline bool GetSourceControl() const {
				return m_PreprocessorLogic.empty() ? true : m_PreprocessorLogic.top();
			}

			// Add
			bool OpenDelimiter(Scripts::Position pos, Delimiter type) {
				auto token = m_Build.CreateToken<Tokens::Delimiter::Info<Delimiter>>(pos, Tokens::Type::Delimiter, pos, Scripts::Range(pos, pos), type);
				m_Delimiters.push(token);
				return true;
			}
			//
			bool CloseDelimiter(Scripts::Position pos, Delimiter type) {
				auto& token = m_Delimiters.top();
				auto tok = token.GetToken<Tokens::Delimiter::Info<Delimiter>>();
				// ensure the delimiters are for the same purpose, otherwise there's error
				if (tok->GetValue<Tokens::Delimiter::DelimiterType>() == type) {
					// replace the token with an updated Scripts::Range
					token = m_Build.CreateToken<Tokens::Delimiter::Info<Delimiter>>(pos, Tokens::Type::Delimiter, pos, Scripts::Range(tok->GetValue<Tokens::Delimiter::ScriptRange>().Begin(), pos), type);
					// mark the closing position
					AddToken<Tokens::Delimiter::Info<Delimiter>>(pos, Tokens::Type::Delimiter, pos, Scripts::Range(tok->GetValue<Tokens::Delimiter::ScriptRange>().Begin(), pos), type);
					m_Delimiters.pop();
					return true;
				}
				return false;
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

			inline Types::Type* GetType(const std::string& name) {
				auto ptr = m_Types.GetType(name);
				return ptr ? ptr : m_Build.GetTypes().GetType(name);
			}

			// Lex main code
			Lexer::Result Lex();
			// Lex with error callback
			template<typename TFunc>
			inline bool Lex(TokenType type, TFunc func) {
				if (Lex() == Lexer::Result::found_token && m_Token == type) {
					return true;
				}
				func(m_Token);
				return false;
			}
			// Lex with custom comparison & error callback
			template<typename TCompFunc, typename TFunc>
			inline bool Lex(TCompFunc compFunc, TFunc func) {
				if (Lex() == Lexer::Result::found_token && compFunc(m_Token)) {
					return true;
				}
				func(m_Token);
				return false;
			}
			// Lex main code, returns true if token of TokenType is found, automatically sends errors
			bool Lexpect(TokenType);
			// Lex around for a number
			bool LexNumber();

			// Add a preprocessing token
			template<typename T, typename... TArgs>
			inline Scripts::Token AddToken(Scripts::Position pos, Tokens::Type token, TArgs&&... args) {
				m_WasLastTokenEOL = false;
				return m_Tokens.Add<T>(pos, token, std::forward<TArgs&&>(args)...);
			}

			// Handle expressions
			int ProcessExpression(bool paren = false);

			// Perform unary operation on passed value - returns false if no change could be made as the operator was unsupported
			static bool ExpressUnary(Operators::Type op, int& val);

			// Get current line number
			inline long GetLineNumber() const { return m_CodePos.GetLine(); }

			// Get code of the current line
			inline CodeLine& GetLineCode() { return m_CodePos.GetLine().GetCode(); }

			// Returns directive_invalid if it didnt exist
			inline Directive GetDirective(const std::string& str) const {
				DirectiveMap::const_iterator it;
				return it = m_Directives.find(str), it != m_Directives.end() ? (*it).second : Directive::INVALID;
			}

			// Returns true if the directive should be processed in a false #if
			inline bool DoesDirectiveIgnoreSourceControl(Directive::Type dir) {
				switch (dir) {
				case Directive::IF:
				case Directive::IFDEF:
				case Directive::IFNDEF:
				case Directive::ELIF:
				case Directive::ELSE:
				case Directive::ENDIF:
					return true;
				}
				return false;
			}
		};

		/*\ Preprocessor::Event - Interesting stuff that the Preprocessor does \*/
		enum class Event
		{
			Begin, Finish,
			Warning,
			Error,
			AddedToken,
			FoundToken,
		};

		/*\ Preprocessor::Task - The Preprocessor and Task become one \*/
		class Task : public TaskSystem::Task<Event>, private Preprocessor
		{
			friend Preprocessor;
			Engine& m_Engine;
			const Information& m_Info;

			inline Engine& GetEngine() { return m_Engine; }

			inline bool operator()(Event id) { return CallEventHandler(id); }

			template<typename... Args>
			inline bool operator()(Event id, Args&&... args) { return CallEventHandler(id, std::forward<Args>(args)...); }

		public:
			Task(Engine& engine, Build* build) : Preprocessor(*this, engine, *build),
				m_Engine(engine), m_Info(GetInfo())
			{ }

			const Information & Info() const { return m_Info; }

			bool IsRunning() const { return Preprocessor::IsRunning(); }
			bool IsTaskFinished() final override { return Preprocessor::IsFinished(); }

		protected:
			void RunTask() final override { Preprocessor::Run(); }
			void ResetTask() final override { Preprocessor::Reset(); }
		};
	}
}