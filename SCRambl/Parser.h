/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include "utils.h"
#include "Engine.h"
#include "Scripts.h"
#include "Preprocessor.h"
#include "Labels.h"
#include "TokensB.h"
#include "Types.h"

namespace SCRambl
{
	namespace Parser
	{
		using Character = Preprocessor::Character;
		using Delimiter = Preprocessor::Delimiter;

		/*\ Parser::Token - Parser wrapper for script tokens \*/
		enum class ParsedType {
			Command, OLCommand,
		};
		/*\ Parser::Symbolic - Symbolic parser data \*/
		enum class SymbolTypes {
			Label, Command, Value
		};
		enum class CommandParam {
			Command = Tokens::Identifier::EXTRA,
		};
		enum States {
			state_neutral, state_parsing_type, state_parsing_command, state_parsing_operator,
			state_parsing_number, state_parsing_label, state_parsing_variable,
			state_parsing_type_varlist, state_parsing_command_args,
			max_state
		};

		using CommandInfo = Tokens::Command::Info<Command>;
		using OLCommandInfo = Tokens::Command::OverloadInfo<Commands::Vector>;

		/*\ Parser::Symbols - Symbolic data for parsed scripts \*/
		class Symbols
		{
		public:
			enum class Structure {
				Header, Data
			};
			enum class DataStructure {

			};

			class Data
			{
				Tokens::ValueToken<Types::Value*>* m_ValToken;
			};

			class FileFunctions
			{
			};

			template<typename T = FileFunctions, typename... TArgs>
			void WriteFile(TArgs&&... args) {
				T();
			}
		};

		/*\ Parser::Error - Errors that can happen while preprocessing \*/
		class Error
		{
		public:
			enum ID {
				// involuntary errors (errors that should be impossible)
				invalid_character = 500,

				// normal errors
				invalid_identifier = 1000,
				invalid_operator,
				label_on_line,
				unsupported_value_type,
				expected_identifier,

				// fatal errors
				fatal_begin = 4000,
				//include_failed = 4000,
				fatal_end,
			};

			Error(ID id) : m_ID(id)
			{ }
			inline operator ID() const			{ return m_ID; }

		private:
			ID			m_ID;
		};

		class Symbolic {
		public:
			inline virtual ~Symbolic()
			{ }
		};

		/*\
		 * Parser::Jump - Jumps from a code position to a label
		\*/
		class Jump : public Symbolic {
			Scripts::Tokens::Iterator m_TokenIt;
			Scripts::Label* m_Dest;

		public:
			using Vector = std::vector<Jump>;

			Jump(Scripts::Label* dest, Scripts::Tokens::Iterator it) :
				m_Dest(dest), m_TokenIt(it)
			{ }
		};

		class Task;

		struct LabelRef {
			Scripts::Tokens::Iterator TokenIt;
			bool IsReference;
			size_t NumUses;

			LabelRef(Scripts::Tokens::Iterator it, bool isref) : TokenIt(it), IsReference(isref)
			{ }
		};

		class Operand
		{
		public:
			enum Type { NullValue, IntValue, FloatValue, TextValue, LabelValue, VariableValue };

			Operand() = default;
			Operand(ScriptVariable* var) : m_Type(VariableValue),
				m_VariableValue(var), m_Text(var->Get().Name())
			{ }
			Operand(ScriptLabel* label) : m_Type(LabelValue),
				m_LabelValue(label), m_Text(label->Get().GetName())
			{ }
			Operand(int64_t v, std::string str) : m_Type(IntValue),
				m_IntValue(v), m_Text(str)
			{ }
			Operand(float v, std::string str) : m_Type(FloatValue),
				m_FloatValue(v), m_Text(str)
			{ }
			Operand(std::string v) : m_Type(TextValue),
				m_Text(v)
			{ }

			Type GetType() const { return m_Type; }

		private:
			Type m_Type = NullValue;
			union {
				int64_t m_IntValue = 0;
				float m_FloatValue;
				ScriptLabel* m_LabelValue;
				ScriptVariable* m_VariableValue;
			};
			std::string m_Text;
		};
		
		/*\
		 * Parser::Operation
		\*/
		class Operation : public Tokens::Symbol
		{
		public:
			enum Type { PrefixUnary, SuffixUnary, Inline, Compounded };
			// PrefixUnary
			Operation(Operators::Operation* op, ScriptVariable* var) : Symbol(Tokens::Type::Operator),
				m_Operation(op), m_Type(PrefixUnary), m_ROperand(var)
			{ }
			// SuffixUnary
			Operation(ScriptVariable* var, Operators::Operation* op) : Symbol(Tokens::Type::Operator),
				m_Operation(op), m_Type(SuffixUnary), m_LOperand(var)
			{ }
			// Inline var + var
			Operation(Operand lop, Operators::Operation* op, Operand rop) : Symbol(Tokens::Type::Operator),
				m_Operation(op), m_Type(Inline), m_LOperand(lop), m_ROperand(rop)
			{ }
			// Compounded
			Operation(Operators::Operation* op, Operand lop, Operand rop) : Symbol(Tokens::Type::Operator),
				m_Operation(op), m_Type(Compounded), m_LOperand(lop), m_ROperand(rop)
			{ }

		private:
			Type m_Type;
			Operand m_LOperand, m_ROperand;
			Operators::Operation* m_Operation;
			bool m_Condition = false;
		};

		/*\
		 * Parser::Parser - Now this is what we're here for
		\*/
		class Parser
		{
			using LabelMap = std::unordered_map<std::string, std::shared_ptr<Scripts::Label>>;

			using DelimiterInfo = Tokens::Delimiter::Info<Delimiter>;
			using CharacterInfo = Tokens::Character::Info<Character>;
			using IdentifierInfo = Tokens::Identifier::Info<>;

			States m_ParseState = state_neutral;
			States m_ActiveState = state_neutral;

			struct ParseState {
				using TMap = std::map<States, ParseState>;
				std::function<bool(ParseState*)> Func;
				TMap Map;

				template<typename TFunc>
				ParseState(TFunc func) : Func(func)
				{ }
				template<typename TFunc>
				ParseState(TFunc func, TMap map) : Func(func), Map(map)
				{ }
			};
			struct TypeParseState {
				struct TypeVarDeclaration {
					Scripts::Tokens::Iterator var_iterator;
					size_t array_size;

					TypeVarDeclaration(Scripts::Tokens::Iterator it) {
						
					}
				};

				Types::Type* type;
				Scripts::Tokens::Iterator type_iterator;
				IToken* token;
				std::vector<TypeVarDeclaration> var_declarations;

				TypeParseState()
				{ }
				TypeParseState(Types::Type* type_, IToken* token_) : type(type_), token(token_)
				{ }
			} m_TypeParseState;
			struct OperationParseState {
				bool looksPrefixed = false;
				bool possiblePostUnary = false;
				bool requireRVal = false;
				ScriptVariable* lh_var = nullptr;
				Types::Type* rh_type = nullptr;
				Operators::Operation* operation = nullptr;

				void StartWithOperator() {
					OperationParseState();
					looksPrefixed = true;
				}
				void StartWithVariable() {
					OperationParseState();
					looksPrefixed = false;
				}
				void HoldPostUnary(Operators::Operation* op, ScriptVariable* var) {
					operation = op;
					lh_var = var;
					possiblePostUnary = true;
				}
				void HoldLHS(ScriptVariable* var) {
					lh_var = var;
					requireRVal = true;
				}
				void FinishRHS(Operators::Operation* op, Types::Type* type) {
					requireRVal = false;
					possiblePostUnary = false;
					looksPrefixed = false;
					operation = op;
					rh_type = type;
				}
				bool CheckForRVal() {
					return possiblePostUnary;
				}
				bool RequireRVal() {
					return requireRVal;
				}
			} m_OperationParseState;
			struct NumberParseState {
				bool ItIsFloat = false;
				union {
					Tokens::Number::Info<Numbers::IntegerType>* IntInfo = nullptr;
					Tokens::Number::Info<Numbers::FloatType>* FloatInfo;
				};
				void Start(Tokens::Number::Info<Numbers::IntegerType>* intInfo) {
					NumberParseState();
					IntInfo = intInfo;
				}
				void Start(Tokens::Number::Info<Numbers::FloatType>* fltInfo) {
					NumberParseState();
					FloatInfo = fltInfo;
					ItIsFloat = true;
				}
				bool IsFloat() const {
					return ItIsFloat;
				}
			} m_NumberParseState;

			States Parse_Neutral();
			States Parse_Neutral_CheckCharacter(IToken*);
			States Parse_Neutral_CheckIdentifier(IToken*);
			States Parse_Neutral_CheckDelimiter(IToken*);
			States Parse_Neutral_CheckOperator(IToken*);
			States Parse_Neutral_CheckNumber(IToken*);
			States Parse_Type();
			States Parse_Type_Varlist();
			States Parse_Command();
			States Parse_Command_Args();
			States Parse_Operator();
			States Parse_Number();
			States Parse_Label();
			States Parse_Variable();

			inline bool IsEOLReached() const {
				return IsCharacterEOL(m_TokenIt->GetToken());
			}
			inline Tokens::Type GetCurrentTokenType() const {
				return m_TokenIt->GetToken()->GetType<Tokens::Type>();
			}

			IToken* PeekToken(Tokens::Type type = Tokens::Type::None, size_t off = 1) {
				auto it = m_TokenIt + off;
				if (it != m_Tokens.End()) {
					auto tok = it->GetToken();
					auto ty = tok->GetType<Tokens::Type>();
					if (type == Tokens::Type::None || type == ty) {
						return tok;
					}
				}
				return nullptr;
			}

			// Sends errors and returns default if fail
			template<typename T>
			static T GetIntegerConstant(IToken* toke, T default_val = 0) {
				auto intinfo = GetIntInfo(toke);
				if (!intinfo)
					BREAK();
				else
					return static_cast<T>(intinfo->GetValue<SCRambl::Tokens::Number::NumberValue>());
				return default_val;
			}

			static Tokens::Number::Info<Numbers::IntegerType>* GetIntInfo(IToken* ptr) {
				return Tokens::Number::IsTypeInt(*ptr) ? static_cast<Tokens::Number::Info<Numbers::IntegerType>*>(ptr) : nullptr;
			}
			static Tokens::Number::Info<Numbers::FloatType>* GetFloatInfo(IToken* ptr) {
				return Tokens::Number::IsTypeFloat(*ptr) ? static_cast<Tokens::Number::Info<Numbers::FloatType>*>(ptr) : nullptr;
			}

			static std::string GetIdentifierName(IToken* toke) {
				auto token = static_cast<Tokens::Identifier::Info<>*>(toke);
				return token->GetValue<Tokens::Identifier::ScriptRange>().Format();
			}
			static std::string GetTextString(IToken* toke) {
				auto token = static_cast<Tokens::String::Info*>(toke);
				return token->GetValue<Tokens::String::ScriptRange>().Format();
			}
			static Scripts::Range GetOperatorRange(IToken* toke) {
				auto tok = static_cast<Tokens::Operator::Info<Operators::OperatorRef>*>(toke);
				return tok->GetValue<Tokens::Operator::ScriptRange>();
			}
			
			static Character GetCharacterValue(IToken* toke) {
				auto token = static_cast<CharacterInfo*>(toke);
				return token->GetValue<Tokens::Character::Parameter::CharacterValue>();
			}
			static bool IsCharacter(IToken* toke) {
				return toke->GetType<Tokens::Type>() == Tokens::Type::Character;
			}
			static bool IsCharacterEOL(IToken* toke) {
				auto tok = static_cast<CharacterInfo*>(toke);
				return IsCharacter(tok) && tok->GetValue<Tokens::Character::Parameter::CharacterValue>() == Character::EOL;
			}
			static bool IsSubscriptDelimiter(IToken* toke) {
				auto info = static_cast<Tokens::Delimiter::Info<Delimiter>*>(toke);
				auto delimtype = info->GetValue<Tokens::Delimiter::Parameter::DelimiterType>();
				return delimtype == Delimiter::Subscript;
			}
			static bool IsScopeDelimiter(IToken* toke) {
				auto info = static_cast<Tokens::Delimiter::Info<Delimiter>*>(toke);
				auto delimtype = info->GetValue<Tokens::Delimiter::Parameter::DelimiterType>();
				return delimtype == Delimiter::Subscript;
			}
			static bool IsScopeDelimiterClosing(IToken* toke) {
				auto tok = static_cast<DelimiterInfo*>(toke);
				return IsScopeDelimiter(toke) && tok->GetValue<Tokens::Delimiter::ScriptRange>().End() == tok->GetValue<Tokens::Delimiter::ScriptPosition>();
			}
			static bool IsOperator(IToken* toke) {
				//return static_cast<TokenBase<Tokens::Type>*>(toke)->GetType() == Tokens::Type::Operator;
				return toke->GetType<Tokens::Type>() == Tokens::Type::Operator;
			}
			static Operators::OperatorRef GetOperator(IToken* toke) {
				auto tok = static_cast<Tokens::Operator::Info<Operators::OperatorRef>*>(toke);
				auto operater = tok->GetValue<Tokens::Operator::OperatorType>();
				return operater;
			}
			static bool IsOperatorConditional(IToken* toke) {
				auto operater = GetOperator(toke);
				return operater && operater->IsConditional();
			}

			bool GetDelimitedArrayIntegerConstant(size_t& i) {
				bool b = false;
				auto next = PeekToken(Tokens::Type::Delimiter);
				if (next) {
					if (!IsSubscriptDelimiter(next)) {
						//SendError();
						BREAK();
					}
					if (next = PeekToken(Tokens::Type::Number, 2)) {
						i = GetIntegerConstant<size_t>(next);
						b = true;
						++m_TokenIt;
					}
					++m_TokenIt;
				}
				return b;
			}

			std::vector<IToken*> m_ParserTokens;
			std::vector<Tokens::Symbol*> m_ParserSymbols;
			template<typename TTokenType, typename... TArgs>
			TTokenType* CreateToken(TArgs&&... args) {
				auto token = new TTokenType(args...);
				m_ParserTokens.emplace_back(token);
				return token;
			}
			template<typename TSymbolType, typename... TArgs>
			TSymbolType* CreateSymbol(TArgs&&... args) {
				auto symbol = new TSymbolType(args...);
				m_ParserSymbols.emplace_back(symbol);
				return symbol;
			}

		public:
			enum State {
				init, parsing, overloading, finished,
				bad_state, max_state = bad_state,
			};

			Parser(Task& task, Engine& engine, Build& build);

			bool IsFinished() const;
			bool IsRunning() const;
			void ParseOverloadedCommand();
			void Run();
			void Reset();

			size_t GetNumTokens() const;
			size_t GetCurrentToken() const;
			Scripts::Token GetToken() const;

		private:
			// Send an error event
			void SendError(Error);
			template<typename First, typename... Args> void SendError(Error, First&&, Args&&...);

			bool ParseCommandOverloads(const Commands::Vector & vec);
			void BeginCommandParsing();
			inline bool IsCommandParsing() const {
				return m_ParsingCommandArgs && m_CurrentCommand;
			}
			inline bool IsOverloading() const {
				return m_State == overloading;
			}
			inline bool AreCommandArgsParsed() const {
				return m_EndOfCommandArgs;
			}
			inline void NextCommandArg() {
				if (m_CurrentCommand) {
					if (!m_EndOfCommandArgs)
					{
						++m_CommandArgIt;
						++m_NumCommandArgs;
					}

					m_EndOfCommandArgs = m_CommandArgIt == m_CurrentCommand->EndArg();
				}
				else m_EndOfCommandArgs = true;
			}
			inline void NextCommandOverload() {
				++m_OverloadCommandsIt;
			}
			void FinishCommandParsing() {
				auto tok = m_CommandTokenIt.Get()->GetToken<Tokens::Command::Info<Command>>();
				auto symbol = m_Build.CreateSymbol<Tokens::Command::Call<Command>>(*tok, m_NumCommandArgs);
				m_CommandTokenIt.Get()->GetSymbol() = symbol;

				size_t cmdid = m_CommandVector.size();
				auto it = m_CommandMap.empty() ? m_CommandMap.end() : m_CommandMap.find(m_CurrentCommand->GetName());
				if (it != m_CommandMap.end()) {
					cmdid = it->second;
				}
				else {
					m_CommandMap.emplace(m_CurrentCommand->GetName(), m_CommandVector.size());
					m_CommandVector.emplace_back(m_CurrentCommand);
				}

				for (auto argtok : m_CommandArgTokens) {
					symbol->AddArg(argtok->GetSymbol());
				}
				m_CommandArgTokens.clear();

				m_Build.GetScript().GetDeclarations().emplace_back(m_Build.CreateSymbol<Tokens::Command::Decl<Command>>(cmdid, m_CurrentCommand));

				m_ParsingCommandArgs = false;
			}
			//Types::Translation<>::Shared FigureOutTranslation()
			void MarkOverloadIncompatible() {
				m_OverloadCommandsIt = m_OverloadCommands.erase(m_OverloadCommandsIt);
			}
			void FailCommandOverload() {
				++m_NumOverloadFailures;
			}
			size_t GetNumberOfOverloadFailures() const {
				return m_NumOverloadFailures;
			}
			void AddLabel(ScriptLabel* label, Scripts::Tokens::Iterator it) {
				m_LabelReferences.emplace(label, LabelRef(it, false));
			}
			void AddLabelRef(ScriptLabel* label, Scripts::Tokens::Iterator it) {
				auto iter = m_LabelReferences.find(label);
				if (iter == m_LabelReferences.end()) {
					m_LabelReferences.emplace(label, LabelRef(it, true));
				}
				else {
					++iter->second.NumUses;
				}
			}

			inline Types::Type* GetType(const std::string& name) {
				auto ptr = m_Types.GetType(name);
				return ptr ? ptr : m_Build.GetTypes().GetType(name);
			}

			void Init();
			void Finish();
			void Parse();

			State m_State = init;
			Engine& m_Engine;
			Task& m_Task;
			Build& m_Build;
			Scripts::Tokens& m_Tokens;
			Scripts::Tokens::Iterator m_TokenIt;
			Scripts::Tokens::Iterator m_VariableTokenIt;
			Scripts::Tokens::Iterator m_CommandTokenIt;
			Scripts::Tokens::Iterator m_OperatorTokenIt;
			Scripts::Tokens::Iterator::CVector m_CommandTokens;			// positions of all parsed command tokens
			Scripts::Tokens::Iterator::CVector m_LabelTokens;
			//Scripts::Labels& m_Labels;
			Types::Types& m_Types;
			size_t m_NumCommandArgs;
			size_t m_NumOverloadFailures;
			ScriptVariable* m_Variable = nullptr;
			Commands& m_Commands;
			Commands m_ExtraCommands;
			Command* m_CurrentCommand;
			Operators::OperatorRef m_CurrentOperator;
			Command::Arg::Iterator m_CommandArgIt;
			Commands::Vector m_OverloadCommands;
			Commands::Vector::iterator m_OverloadCommandsIt;
			Jump::Vector m_Jumps;

			std::map<ScriptLabel*, LabelRef> m_LabelReferences;
			std::vector<std::shared_ptr<const Command>> m_CommandVector;
			std::unordered_map<std::string, size_t> m_CommandMap;
			std::multimap<const std::string, Scripts::Tokens::Iterator> m_CommandTokenMap;

			std::vector<Scripts::Token*> m_CommandArgTokens;

			// Status
			bool m_OnNewLine;
			bool m_ParsingCommandArgs;
			bool m_EndOfCommandArgs;
		};
		
		/*\
		 * Parser::Event - Interesting stuff that the Preprocessor does
		\*/
		enum class Event
		{
			Begin, Finish,
			Warning,
			Error,
			FoundToken,
		};
		
		/*\ Parser::Task \*/
		class Task : public TaskSystem::Task<Event>, private Parser
		{
			friend Parser;
			Engine&	m_Engine;

			inline bool operator()(Event id) { return CallEventHandler(id); }
			template<typename... Args>
			inline bool operator()(Event id, Args&&... args) { return CallEventHandler(id, std::forward<Args>(args)...); }

		public:
			Task(Engine& engine, Build* build) :
				Parser(*this, engine, *build),
				m_Engine(engine)
			{ }

			inline size_t GetProgressCurrent() const { return GetCurrentToken(); }
			inline size_t GetProgressTotal() const { return GetNumTokens(); }
			inline Scripts::Token GetToken() const { return Parser::GetToken(); }

			bool IsRunning() const { return Parser::IsRunning(); }
			bool IsTaskFinished() final override { return Parser::IsFinished(); }

		protected:
			void RunTask() final override { Parser::Run(); }
			void ResetTask() final override { Parser::Reset(); }
		};
	}
}