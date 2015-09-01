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
#include "Labels.h"
#include "TokenInfo.h"
#include "Tokens.h"

namespace SCRambl
{
	namespace Preprocessing
	{
		// Task is a class
		class Task;
		class Error;
		
		// interesting stuff what the Preprocessor does
		/*enum class Event {
			Begin, Finish,
			Warning,
			Error,
			AddedToken,
			FoundToken,
		};*/
		
		// Lexing::Scanner for nothingness (useless?)
		class WhitespaceScanner : public Lexing::Scanner {
		public:
			bool Scan(Lexing::State& state, Scripts::Position& code) override;
		};
		// Lexing::Scanner for identifiers
		class IdentifierScanner : public Lexing::Scanner {
		public:
			bool Scan(Lexing::State& state, Scripts::Position& pos) override;
		};
		// Lexing::Scanner for labels
		class LabelScanner : public Lexing::Scanner {
		public:
			bool Scan(Lexing::State& state, Scripts::Position& pos) override;
		};
		// Lexing::Scanner for directives
		class DirectiveScanner : public Lexing::Scanner {
		public:
			bool Scan(Lexing::State& state, Scripts::Position& pos) override;
		};
		// Lexing::Scanner for string literals
		class StringLiteralScanner : public Lexing::Scanner {
		public:
			enum class Error {
				unterminated
			};

			bool Scan(Lexing::State& state, Scripts::Position& pos) override;
		};
		// Lexing::Scanner for line comments
		class CommentScanner : public Lexing::Scanner {
		public:
			bool Scan(Lexing::State& state, Scripts::Position& pos) override;
		};
		// Lexing::Scanner for block comments
		class BlockCommentScanner : public Lexing::Scanner {
			int depth = 0;

		public:
			enum class Error {
				end_of_file_reached,
			};

			bool Scan(Lexing::State& state, Scripts::Position& pos) override;
		};
		
		// Externally accessible info about the preprocessors current state
		class Information {
			Scripts::Position& m_ScriptPosition;

			void SetScriptPos(Scripts::Position& pos) { m_ScriptPosition = pos; }

		public:
			Information(Scripts::Position& pos) : m_ScriptPosition(pos)
			{ }

			inline const Scripts::Position& GetScriptPos() const { return m_ScriptPosition; }
		};
		// Errors that can happen while preprocessing
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
		
		// Preprocessor directive stuff
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
			Type m_Type = INVALID;
		};
		// Preprocessor character stuff
		class Character {
		public:
			enum Type { EOL, Colonnector, Conditioner };

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
				case Conditioner: rep = "?";
					break;
				}
				return !rep.empty() ? rep : "[UNK]";
			}

		private:
			Type m_Type;
		};
		// Preprocessor delimiter stuff
		class Delimiter {
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

		template<typename T, Numbers::Type TType>
		class TokenNumber : public Tokens::Number::Info<T> {
			using Parent = Tokens::Number::Info<T>;
			T m_Val;

		public:
			TokenNumber(Scripts::Range rg, T val) : Parent(Tokens::Type::Number, rg, TType, &m_Val),
				m_Val(val)
			{ }
		};
		class TokenDelimiter : public Tokens::Delimiter::Info<Delimiter> {
			using Parent = Tokens::Delimiter::Info<Delimiter>;

		public:
			TokenDelimiter(Scripts::Position pos, Scripts::Range range, Delimiter type) : Parent(Tokens::Type::Delimiter, pos, range, &m_Delimiter),
				m_Delimiter(type)
			{ }

		private:
			Delimiter m_Delimiter = Delimiter::None;
		};

		struct event : public build_event {
			explicit event(const Engine& engine) : build_event(engine)
			{ }
		};
		struct event_begin : public event {
			explicit event_begin(const Engine& engine) : event(engine)
			{ LinkEvent<event_begin>("event_begin"); }
		};
		struct event_finish : public event {
			explicit event_finish(const Engine& engine) : event(engine)
			{ LinkEvent<event_finish>("event_finish"); }
		};
		struct event_warning : public event {
			explicit event_warning(const Engine& engine) : event(engine)
			{ LinkEvent<event_warning>("event_warning"); }
		};
		template<Error::ID TID, typename... TArgs>
		struct event_error : public error_event_data<TArgs...> {
			event_error(const Engine& engine, TArgs... args) : error_event_data(Basic::Error(engine, TID), std::forward<TArgs>(args)...)
			{ LinkEvent<event_warning>("event_warning"); }
		};
		using error_include_failed							= event_error<Error::include_failed, std::string>;
		using error_dir_expected_file_name					= event_error<Error::dir_expected_file_name, Directive>;
		using error_dir_expected_command_id					= event_error<Error::dir_expected_command_id, Directive>;

		struct event_found_token : public token_event {
			using token_event::token_event;

			event_found_token(const Engine& engine, Scripts::Range rg) : token_event(engine, rg)
			{ }
		};

		// Main Preprocessor task routine
		class Preprocessor {
			friend class Information;

			using LexerToken = Lexing::Token<TokenType>;
			using LexerMachine = Lexing::Lexer<TokenType>;
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

			void Run();
			void Reset();
			inline State GetState() const { return m_State; }
			inline bool IsRunning() const { return GetState() != finished; }
			inline bool IsFinished() const { return GetState() == finished; }

		protected:
			const Information& GetInfo() const { return m_Information; }

		private:
			// Send an error event
			template<typename... TArgs>
			void SendError(Error, TArgs&&... args);
			template<typename First, typename... Args>
			void SendError(Error, First&&, Args&&...);

			// Enter conditional source compilation
			void PushSourceControl(bool);
			// Return from conditional source compilation
			void PopSourceControl();
			// Invert conditional source compilation state
			void InvertSourceControl();
			// Get conditional source compilation state
			bool GetSourceControl() const;

			// Starte de limiting
			bool OpenDelimiter(Scripts::Position, Delimiter);
			// Ende de limiting
			bool CloseDelimiter(Scripts::Position, Delimiter);

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
			// Get type (including added script types)
			VecRef<Types::Type> GetType(const std::string&);
			// Lex main code
			Lexing::Result Lex();
			// Lex with error callback
			template<typename TFunc>
			inline bool Lex(TokenType type, TFunc func) {
				if (Lex() == Lexing::Result::found_token && m_Token == type) {
					return true;
				}
				func(m_Token);
				return false;
			}
			// Lex with custom comparison & error callback
			template<typename TCompFunc, typename TFunc>
			inline bool Lex(TCompFunc compFunc, TFunc func) {
				if (Lex() == Lexing::Result::found_token && compFunc(m_Token)) {
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
			inline VecRef<Tokens::Token> AddToken(Scripts::Position pos, TArgs&&... args) {
				m_WasLastTokenEOL = false;
				return m_Tokens.Add<T>(pos, std::forward<TArgs&&>(args)...);
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

		private:
			// basic necessities
			State m_State = init;
			Information m_Information;
			Engine&	m_Engine;
			Build& m_Build;
			Task& m_Task;
			Commands& m_Commands;
			Tokens::Storage& m_Tokens;
			//
			Scripts::Position m_CodePos;
			//
			std::string m_String;					// last scanned string
			std::string	m_Identifier;				// last scanned identifier
			Directive m_Directive;
			//
			LexerToken m_Token;
			LexerMachine m_Lexer;
			MacroMap m_Macros;
			DirectiveMap m_Directives;
			std::stack<VecRef<Tokens::Token>> m_Delimiters;
			//
			bool m_DisableMacroExpansion = false;
			bool m_DisableMacroExpansionOnce = false;
			bool m_WasLastTokenEOL = false;
			std::stack<bool> m_PreprocessorLogic;
			// our precious scanners
			BlockCommentScanner m_BlockCommentScanner;
			CommentScanner m_CommentScanner;
			DirectiveScanner m_DirectiveScanner;
			IdentifierScanner m_IdentifierScanner;
			Numbers::Scanner m_NumericScanner;
			LabelScanner m_LabelScanner;
			StringLiteralScanner m_StringLiteralScanner;
			WhitespaceScanner m_WhitespaceScanner;
			// operator scanners are super special
			OperatorTable m_Operators;
			OperatorScanner m_OperatorScanner;
			Operators::Table<Operators::OperatorRef>& m_ParserOperators;
			Operators::Scanner<Operators::OperatorRef> m_ParserOperatorScanner;
		};

		// The Preprocessor and Task become one
		class Task : public TaskSystem::Task, private Preprocessor {
			friend Preprocessor;

		public:
			Task(Engine&, Build*);

			const Information& Info() const;

			bool IsRunning() const;
			bool IsTaskFinished() const final override;

			template<typename TEvent, typename... TArgs>
			inline size_t Event(TArgs&&... args) {
				return CallEvent(TEvent(m_Engine, std::forward<TArgs>(args)...));
			}

		protected:
			void RunTask() final override;
			void ResetTask() final override;

		private:
			Engine& m_Engine;
			const Information& m_Info;
		};
	}
}