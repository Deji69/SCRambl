/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include "utils.h"
#include "Standard.h"
#include "Engine.h"
#include "Scripts.h"
#include "Preprocessor.h"
#include "Labels.h"
#include "Tokens.h"
#include "TokensB.h"
#include "TokenInfo.h"
#include "Operands.h"

namespace SCRambl
{
	namespace Parsing
	{
		using Character = Preprocessing::Character;
		using Delimiter = Preprocessing::Delimiter;

		// Interesting stuff that the parser does
		enum class Event {
			Begin, Finish,
			Warning, Error,
			FoundToken,
		};
		// Parser states
		enum States {
			state_neutral, state_parsing_type, state_parsing_command, state_parsing_operator,
			state_parsing_number, state_parsing_string, state_parsing_label, state_parsing_variable,
			state_parsing_subscript, state_parsing_type_varlist, state_parsing_type_command,
			state_parsing_command_args,
			max_state
		};

		class Task; class Parser;
		using CommandInfo = Tokens::Command::Info;
		using OLCommandInfo = Tokens::Command::OverloadInfo;

		// Symbolic data for parsed scripts
		class Symbols
		{
		public:
			enum class Structure {
				Header, Data
			};
			enum class DataStructure {

			};

			class Data {
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
		// Errors that can happen while parsing
		class Error {
		public:
			enum ID {
				// involuntary errors (errors that should be impossible!!)
				invalid_character = 500,

				// normal errors
				invalid_identifier = 1000,
				invalid_operator,
				invalid_command,
				label_on_line,
				unsupported_value_type,
				expected_identifier,			// expected an identifier (_id1/i_d1/_1id)
				expected_key_identifier,		// expected a key identifier (identifier/number/"string")
				expected_colon_punctuator,		// expected a :
				expected_integer_constant,

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

		// Parameter
		struct Parameter {
			Operand operand;
			Types::Value* value;

			Parameter(Operand op, Types::Value* val) : operand(op), value(val)
			{ }
		};
		// Chain operation
		class ChainOperation {
		public:
			ChainOperation(Operators::OperatorRef op, Parameter param) : m_Operator(op), m_Parameter(param)
			{ }

			inline Operators::OperatorRef GetOperator() const { return m_Operator; }
			inline Parameter& GetOperand() { return m_Parameter; }

		private:
			Operators::OperatorRef m_Operator;
			Parameter m_Parameter;
		};
		// Operation symbol
		class Operation //: public TokenSymbol
		{
		public:
			enum Type { PrefixUnary, SuffixUnary, Inline, Compounded };
			// PrefixUnary
			Operation(Operators::OperationRef op, ScriptVariable* var) : //Symbol(Tokens::Type::Operator),
				m_Operation(op), m_Type(PrefixUnary), m_ROperand(var)
			{ }
			// SuffixUnary
			Operation(ScriptVariable* var, Operators::OperationRef op) : //Symbol(Tokens::Type::Operator),
				m_Operation(op), m_Type(SuffixUnary), m_LOperand(var)
			{ }
			// Inline var + var
			Operation(Operand lop, Operators::OperationRef op, Operand rop) : //Symbol(Tokens::Type::Operator),
				m_Operation(op), m_Type(Inline), m_LOperand(lop), m_ROperand(rop)
			{ }
			// Compounded
			Operation(Operators::OperationRef op, Operand lop, Operand rop) : //Symbol(Tokens::Type::Operator),
				m_Operation(op), m_Type(Compounded), m_LOperand(lop), m_ROperand(rop)
			{ }

		private:
			Type m_Type;
			Operand m_LOperand, m_ROperand;
			Operators::OperationRef m_Operation;
			bool m_Condition = false;
		};
		// Command call symbol
		class CommandCall : public Tokens::Symbol {
		public:
			CommandCall(Types::Translation::Ref translation, CommandAttributes attributes) : Tokens::Symbol(Tokens::Type::CommandCall),
				m_Translation(translation), m_Attributes(attributes)
			{ }

		private:
			Types::Translation::Ref m_Translation;
			CommandAttributes m_Attributes;
		};
		// Label reference
		struct LabelRef {
			Tokens::Iterator TokenIt;
			bool IsReference;
			size_t NumUses;

			LabelRef(Tokens::Iterator it, bool isref) : TokenIt(it), IsReference(isref)
			{ }
		};
		
		// The parser
		class Parser {
			using DelimiterInfo = Tokens::Delimiter::Info<Delimiter>;
			using CharacterInfo = Tokens::Character::Info<Character>;
			using IdentifierInfo = Tokens::Identifier::Info<>;

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
					Tokens::Iterator var_iterator;
					size_t array_size;

					TypeVarDeclaration(Tokens::Iterator it) : var_iterator(it)
					{ }
				};

				Types::Type* type;
				Tokens::Iterator type_iterator;
				IToken* token;
				std::vector<TypeVarDeclaration> var_declarations;

				TypeParseState()
				{ }
				TypeParseState(Types::Type* type_, IToken* token_) : type(type_), token(token_)
				{ }
			} m_TypeParseState;
			struct CommandParseState {
				VecRef<Command> command;
				CommandArg::Iterator commandArgIt;
				std::vector<Tokens::CommandArgs::Arg> args;

				CommandParseState() = default;
				void Begin(VecRef<Command> cmd, CommandArg::Iterator argit) {
					CommandParseState();
					command = cmd;
					commandArgIt = argit;
				}
				Tokens::CommandArgs::Arg* AddParameter(Operand op, Types::Value* value) {
					args.emplace_back(op, value);
					return &args.back();
				}
				
			} m_CommandParseState;
			struct OperationParseState {
				bool looksPrefixed = false;
				bool possiblePostUnary = false;
				bool requireRVal = false;
				bool inChain = false;
				ScriptVariable* lh_var = nullptr;
				Types::Type* rh_type = nullptr;
				Operators::OperatorRef baseOperator;
				Operators::OperationRef operation;
				std::vector<ChainOperation> chainedOps;

				void StartWithOperator(Operators::OperatorRef op) {
					OperationParseState();
					looksPrefixed = true;
					baseOperator = op;
				}
				void StartWithVariable() {
					OperationParseState();
					looksPrefixed = false;
				}
				void HoldPostUnary(Operators::OperationRef op, ScriptVariable* var) {
					baseOperator = op->GetOperator();
					operation = op;
					lh_var = var;
					possiblePostUnary = true;
				}
				void HoldLHS(Operators::OperatorRef op, ScriptVariable* var) {
					baseOperator = op;
					lh_var = var;
					requireRVal = true;
				}
				void FinishRHS(Operators::OperationRef op, Types::Type* type) {
					requireRVal = false;
					possiblePostUnary = false;
					looksPrefixed = false;
					operation = op;
					rh_type = type;
				}
				void PrepareChain() {
					inChain = true;
					requireRVal = false;
				}
				ChainOperation& Chain(Operators::OperatorRef op, Parameter param) {
					chainedOps.emplace_back(op, param);
					return chainedOps.back();
				}
				ChainOperation& LastInChain() {
					return chainedOps.back();
				}
				
				bool CheckForRVal() {
					return possiblePostUnary;
				}
				bool RequireRVal() {
					return requireRVal;
				}
				bool IsInChain() {
					return inChain;
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
			Tokens::Token GetToken() const;

		private:
			States Parse_Neutral();
			States Parse_Neutral_CheckCharacter(IToken*);
			States Parse_Neutral_CheckIdentifier(IToken*);
			States Parse_Neutral_CheckDelimiter(IToken*);
			States Parse_Neutral_CheckOperator(IToken*);
			States Parse_Neutral_CheckNumber(IToken*);
			States Parse_Neutral_CheckString(IToken*);
			States Parse_Type();
			States Parse_Type_Varlist();
			States Parse_Type_CommandDef();
			States Parse_Command();
			States Parse_Command_Arglist();
			States Parse_Operator();
			States Parse_Number();
			States Parse_String();
			States Parse_Label();
			States Parse_Variable();
			States Parse_Subscript();

			inline bool IsEOLReached() const {
				return m_TokenIt == m_Tokens.end() || IsCharacterEOL(m_TokenIt->GetToken());
			}
			inline Tokens::Type GetCurrentTokenType() const {
				return m_TokenIt->GetToken()->GetType<Tokens::Type>();
			}
			inline Types::Value* GetBestValue(Types::ValueSet type, size_t size) {
				Types::Value* best_value = nullptr;
				m_Build.GetTypes().AllValues(type, [size, &best_value](Types::Value* value){
					if (value->CanFitSize(size)) {
						if (!best_value || best_value->GetSize() > value->GetSize())
							best_value = value;
					}
					return false;
				});
				return best_value;
			}

			IToken* PeekToken(Tokens::Type type = Tokens::Type::None, size_t off = 1);

			// Sends errors and returns default if fail
			template<typename T>
			T GetIntegerConstant(IToken* toke, T default_val = 0) {
				auto intinfo = GetIntInfo(toke);
				if (!intinfo) {
					SendError(Error::expected_integer_constant);
				}
				else
					return static_cast<T>(*intinfo->GetValue<SCRambl::Tokens::Number::NumberValue>());
				return default_val;
			}

			static Tokens::Number::Info<Numbers::IntegerType>* GetIntInfo(IToken* ptr) {
				return Tokens::Number::IsTypeInt(*ptr) ? static_cast<Tokens::Number::Info<Numbers::IntegerType>*>(ptr) : nullptr;
			}
			static Tokens::Number::Info<Numbers::FloatType>* GetFloatInfo(IToken* ptr) {
				return Tokens::Number::IsTypeFloat(*ptr) ? static_cast<Tokens::Number::Info<Numbers::FloatType>*>(ptr) : nullptr;
			}

			static Tokens::Type GetTokenType(IToken* toke) {
				return toke->GetType<Tokens::Type>();
			}
			static bool IsTokenType(IToken* toke, Tokens::Type type) {
				return toke && GetTokenType(toke) == type;
			}
			static std::string GetIdentifierName(IToken* toke) {
				return Tokens::Identifier::GetScriptRange(*toke).Format();
			}
			static std::string GetTextString(IToken* toke) {
				auto token = static_cast<Tokens::String::Info*>(toke);
				return token->GetValue<Tokens::String::ScriptRange>().Format();
			}
			static Scripts::Range GetOperatorRange(IToken* toke) {
				auto tok = static_cast<Tokens::Operator::Info<Operators::OperatorRef>*>(toke);
				return tok->GetValue<Tokens::Operator::ScriptRange>();
			}
			static std::string GetTokenString(IToken* toke) {
				if (toke) {
					auto type = toke->GetType<Tokens::Type>();
					switch (type) {
					case Tokens::Type::Identifier:
						return Tokens::Identifier::GetScriptRange(*toke).Format();
					case Tokens::Type::Number:
						return Tokens::Number::GetScriptRange(*toke).Format();
					case Tokens::Type::String:
						return Tokens::String::GetString(*toke);
					default:
						BREAK(); // ?
					}
				}
				return "";
			}

			static Character GetCharacterValue(IToken* toke) {
				return Tokens::Character::GetCharacter<Character>(*toke);
			}
			static bool IsCharacter(IToken* toke) {
				return toke->GetType<Tokens::Type>() == Tokens::Type::Character;
			}
			static bool IsCharacterEOL(IToken* toke) {
				return IsCharacter(toke) && GetCharacterValue(toke) == Character::EOL;
			}
			static bool IsSubscriptDelimiter(IToken* toke) {
				return Tokens::Delimiter::GetDelimiterType<Delimiter>(*toke) == Delimiter::Subscript;
			}
			static bool IsSubscriptDelimiterClosing(IToken* toke) {
				return IsSubscriptDelimiter(toke) && Tokens::Delimiter::GetScriptRange(*toke).End() == Tokens::Delimiter::GetScriptPosition(*toke);
			}
			static bool IsScopeDelimiter(IToken* toke) {
				return Tokens::Delimiter::GetDelimiterType<Delimiter>(*toke) == Delimiter::Scope;
			}
			static bool IsScopeDelimiterClosing(IToken* toke) {
				return IsScopeDelimiter(toke) && Tokens::Delimiter::GetScriptRange(*toke).End() == Tokens::Delimiter::GetScriptPosition(*toke);
			}
			static bool IsOperator(IToken* toke) {
				return toke->GetType<Tokens::Type>() == Tokens::Type::Operator;
			}
			static bool IsOperatorConditional(IToken* toke) {
				auto operater = GetOperator(toke);
				return operater && operater->IsConditional();
			}
			static bool IsConditionPunctuator(IToken* toke) {
				return IsTokenType(toke, Tokens::Type::Character) && GetCharacterValue(toke) == Character::Conditioner;
			}
			static bool IsColonPunctuator(IToken* toke) {
				return IsTokenType(toke, Tokens::Type::Character) && GetCharacterValue(toke) == Character::Colonnector;
			}
			static Operators::OperatorRef GetOperator(IToken* toke) {
				auto tok = static_cast<Tokens::Operator::Info<Operators::OperatorRef>*>(toke);
				auto operater = tok->GetValue<Tokens::Operator::OperatorType>();
				return operater;
				//return Tokens::Operator::GetOperator<Operators::OperatorRef>(*toke);
			}

			bool GetDelimitedArrayIntegerConstant(size_t&);

			void EnterSubscript(IToken *token) {
				m_Subscripts.emplace_back(token);
			}
			void LeaveSubscript() {
				m_Subscripts.erase(m_Subscripts.end());
			}
			size_t GetSubscriptDepth() const {
				return m_Subscripts.size();
			}

			std::vector<IToken*> m_ParserTokens;
			std::set<Tokens::CommandArgs::Info*> m_CommandArgsSet;
			template<typename TTokenType, typename... TArgs>
			TTokenType* CreateToken(TArgs&&... args) {
				auto token = new TTokenType(args...);
				m_ParserTokens.emplace_back(token);
				return token;
			}
			
		private:
			// Send an error event
			inline void SendError(Error);
			// Send an error event (with args)
			template<typename First, typename... Args>
			inline void SendError(Error, First&&, Args&&...);

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
				auto tok = m_CommandTokenIt.Get().GetToken<Tokens::Command::Info>();
				//auto symbol = m_Build.CreateSymbol<Tokens::Command::Call>(*tok, m_NumCommandArgs);
				//m_CommandTokenIt.Get().GetSymbol() = symbol;

				size_t cmdid = m_CommandVector.size();
				auto it = m_CommandMap.empty() ? m_CommandMap.end() : m_CommandMap.find(m_CurrentCommand->Name());
				if (it != m_CommandMap.end()) {
					cmdid = it->second;
				}
				else {
					m_CommandMap.emplace(m_CurrentCommand->Name(), m_CommandVector.size());
					m_CommandVector.emplace_back(m_CurrentCommand);
				}

				for (auto argtok : m_CommandArgTokens) {
					//symbol->AddArg(argtok->GetSymbol());
				}
				m_CommandArgTokens.clear();

				//m_Build.GetDeclarations().emplace_back(m_Build.CreateSymbol<Tokens::Command::Decl>(cmdid, m_CurrentCommand));

				m_ParsingCommandArgs = false;
			}
			void MarkOverloadIncompatible() {
				m_OverloadCommandsIt = m_OverloadCommands.erase(m_OverloadCommandsIt);
			}
			void FailCommandOverload() {
				++m_NumOverloadFailures;
			}
			size_t GetNumberOfOverloadFailures() const {
				return m_NumOverloadFailures;
			}
			void AddLabel(ScriptLabel* label, Tokens::Iterator it) {
				m_LabelReferences.emplace(label, LabelRef(it, false));
			}
			void AddLabelRef(ScriptLabel* label, Tokens::Iterator it) {
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
			States m_ActiveState = state_neutral;
			States m_ParseState = state_neutral;
			Engine& m_Engine;
			Task& m_Task;
			Build& m_Build;
			Tokens::Storage& m_Tokens;
			Tokens::Iterator m_TokenIt;
			Tokens::Iterator m_VariableTokenIt;
			Tokens::Iterator m_CommandTokenIt;
			Tokens::Iterator m_OperatorTokenIt;
			std::vector<const Tokens::Iterator> m_CommandTokens;			// positions of all parsed command tokens
			std::vector<const Tokens::Iterator> m_LabelTokens;
			Types::Types& m_Types;
			size_t m_NumCommandArgs;
			size_t m_NumOverloadFailures;
			ScriptVariable* m_Variable = nullptr;
			Commands& m_Commands;
			Commands m_ExtraCommands;
			VecRef<Command> m_CurrentCommand;
			Operators::OperatorRef m_CurrentOperator;
			Command::Arg::Iterator m_CommandArgIt;
			Commands::Vector m_OverloadCommands;
			Commands::Vector::iterator m_OverloadCommandsIt;
			VecRef<Types::Xlation> m_Xlation;

			std::map<ScriptLabel*, LabelRef> m_LabelReferences;
			std::vector<VecRef<Command>> m_CommandVector;
			std::unordered_map<std::string, size_t> m_CommandMap;
			std::multimap<const std::string, Tokens::Iterator> m_CommandTokenMap;
			std::vector<IToken*> m_Subscripts;

			std::vector<Tokens::Token*> m_CommandArgTokens;

			// Status
			bool m_OnNewLine;
			bool m_ParsingCommandArgs;
			bool m_EndOfCommandArgs;
		};
		// The parser task
		class Task : public TaskSystem::Task<Event>, private Parser {
			friend Parser;

		public:
			Task(Engine&, Build*);

			size_t GetProgressCurrent() const;
			size_t GetProgressTotal() const;
			Tokens::Token GetToken() const;

			bool IsRunning() const;
			bool IsTaskFinished() final override;

		protected:
			void RunTask() final override;
			void ResetTask() final override;
			// Call event
			inline bool operator()(Event id) { return CallEventHandler(id); }
			// Call event with args
			template<typename... Args>
			inline bool operator()(Event id, Args&&... args) { return CallEventHandler(id, std::forward<Args>(args)...); }

		private:
			Engine&	m_Engine;
		};
	}
}