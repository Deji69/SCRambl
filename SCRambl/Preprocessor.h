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

namespace SCRambl
{
	namespace Preprocessor
	{
		/*\
		 * Preprocessor::Token - Types of token information
		\*/
		class Token {
		public:
			enum class Type {
				None, Directive, Identifier, Label, Number, Operator, String, Character
			};

			class None {
			public:
				static const enum Value { TokenType };
				using Info = TokenInfo < Type > ;
			};
			class Directive {
			public:
				static const enum Value { TokenType, ScriptRange };
				using Info = TokenInfo < Type, Script::Range >;
			};
			class Identifier {
			public:
				static const enum Value { TokenType, ScriptRange };
				template<typename TType = Type, typename ...TData>
				using Info = TokenInfo < TType, Script::Range, TData... >;
			};
			class Label {
			public:
				static const enum Value { ScriptRange, LabelValue };
				using Info = TokenInfo < Type, Script::Range, Script::Label::Shared >;
			};
			class Number {
			public:
				static const enum Value { ScriptRange, ValueType, NumberValue };
				template<typename TNumberType>
				using Info = TokenInfo < Type, Script::Range, Numbers::Type, TNumberType >;
				using TypelessInfo = TokenInfo < Type, Script::Range, Numbers::Type >;

				static Numbers::Type GetValueType(const IToken& token) {
					return token.Get<const TypelessInfo>().GetValue<ValueType>();
				}
			};
			class Operator {
			public:
				static const enum Value { ScriptRange, OperatorType };
				template<typename TOperatorType>
				using Info = TokenInfo < Type, Script::Range, TOperatorType >;
			};
			class String {
			public:
				static const enum Value { ScriptRange, StringValue };
				using Info = TokenInfo < Type, Script::Range, std::string >;
			};
			class Character {
			public:
				static const enum Value { ScriptPosition, CharacterValue };
				template<typename TCharacterType>
				using Info = TokenInfo < Type, Script::Position, TCharacterType >;
			};

			//using NoneInfo = TokenInfo < Type >;
			//using DirectiveInfo = TokenInfo < PreprocessingToken::Type, Script::Range >;
			//using DirectiveInfo = TokenInfo < Type, Script::Range >;
			//template<typename TType = Type, typename ...TData>
			//using IdentifierInfo = TokenInfo < TType, Script::Range, TData... >;
			//using LabelInfo = TokenInfo < Type, Script::Range, Script::Label::Shared >;
			//template<typename TNumberType>
			//using NumberInfo = TokenInfo < Type, Script::Range, NumberType, TNumberType >;
			//using UnknownNumberInfo = TokenInfo < Type, Script::Range, NumberType > ;
			//template<typename TOperatorType>
			//using OperatorInfo = TokenInfo < Type, Script::Range, TOperatorType >;
			//using StringInfo = TokenInfo < Type, Script::Range, std::string >;
			//template<typename TCharacterType>
			//using CharacterInfo = TokenInfo < Type, Script::Position, TCharacterType >;
		};

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
		 * LabelScanner - Lexer::Scanner for labels
		\*/
		class LabelScanner : public Lexer::Scanner
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
				dir_expected_file_name,					// 1011

				// fatal errors
				fatal_begin								= 4000,
				include_failed							= 4000,
				fatal_end,
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
		};

		/*\
		 - Preprocessor::Character - Preprocessor character stuff
		\*/
		class Character
		{
		public:
			enum Type {
				EOL,
			};

		private:
			Type		m_Type;

		public:
			Character() = default;
			Character(Type type) : m_Type(type)
			{ }

			inline operator Type() const		{ return m_Type; }

			static inline std::string Formatter(Character type) {
				std::string rep;
				switch (type) {
				case EOL: rep = "[EOL]";
					break;
				}
				return !rep.empty() ? rep : "[UNK]";
			}
		};

		/*\
		 - Preprocessor::Preprocessor - Main Preprocessor task routine
		\*/
		class Preprocessor
		{
			friend class Information;

		private:
			using LexerToken = Lexer::Token < TokenType >;
			using LexerMachine = Lexer::Lexer < TokenType >;
			using DirectiveMap = std::unordered_map < std::string, Directive >;
			using OperatorTable = Operator::Table < Operator::Type, Operator::max_operator >;
			using OperatorScanner = Operator::Scanner < Operator::Type, Operator::max_operator >;
			template<typename... T>
			using TToken = TokenInfo < Token::Type, T... > ;

			Engine					&	m_Engine;
			Task					&	m_Task;
			Information					m_Information;

			BlockCommentScanner			m_BlockCommentScanner;
			CommentScanner				m_CommentScanner;
			DirectiveScanner			m_DirectiveScanner;
			IdentifierScanner			m_IdentifierScanner;
			Numbers::Scanner			m_NumericScanner;
			LabelScanner				m_LabelScanner;
			StringLiteralScanner		m_StringLiteralScanner;
			WhitespaceScanner			m_WhitespaceScanner;

			OperatorTable				m_Operators;
			OperatorScanner				m_OperatorScanner;

			LexerMachine				m_Lexer;
			LexerToken					m_Token;
			DirectiveMap				m_Directives;
			Directive					m_Directive = Directive::INVALID;
			std::string					m_String;					// last scanned string
			std::string					m_Identifier;				// last scanned identifier
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
			Script::Tokens					&	m_Tokens;
			Script::Position					m_CodePos;
			bool								m_bScriptIsLoaded;						// if so, we only need to add-in any #include's
			bool								m_DisableMacroExpansion = false;
			bool								m_DisableMacroExpansionOnce = false;
			bool								m_WasLastTokenEOL = false;
			std::stack<bool>					m_PreprocessorLogic;
			unsigned long						m_FalseLogicDepth = 0;					// for #if..#endif's ocurring within "#if FALSE"
			std::unordered_set<std::string>		m_IdentifiersThatAreNotMacros;
			
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

			// Lex main code
			Lexer::Result Lex();
			// Lex around for a number
			bool LexNumber();

			// Add a preprocessing token
			template<typename T, typename... TArgs>
			inline Script::Token::Shared AddToken(Script::Position pos, Token::Type token, TArgs&&... args)
			{
				m_WasLastTokenEOL = false;
				return m_Tokens.Add < T >(pos, token, std::forward<TArgs>(args)...);
			}
			/*template<typename... TArgs>
			inline std::shared_ptr<Token<TArgs...>> AddToken(PreprocessingToken::Type token, TArgs... args)
			{
				return m_Tokens.Add < Token<TArgs...> >(token, std::forward<TArgs>(args)...);
			}*/

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

		/*\
		 * Preprocessor::Event - Interesting stuff that the Preprocessor does
		\*/
		enum class Event
		{
			Begin, Finish,
			Warning,
			Error,
			AddedToken,
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
			{ }

			const Information & Info() const		{ return m_Info; }

		protected:
			bool IsTaskFinished() final override	{ return Preprocessor::IsFinished(); }
			void RunTask() final override			{ Preprocessor::Run(); }
			void ResetTask() final override			{ Preprocessor::Reset(); }
		};
	}
}