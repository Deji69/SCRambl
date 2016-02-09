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
#include "Operators.h"

namespace SCRambl
{
	namespace Parsing
	{
		using Character = Preprocessing::Character;
		using Delimiter = Preprocessing::Delimiter;

		// Interesting stuff that the parser does
		/*enum class Event {
			Begin, Finish,
			Warning, Error,
			FoundToken,
		};*/
		// Parser states
		enum States {
			state_neutral, state_parsing_type, state_parsing_command, state_parsing_operator,
			state_parsing_number, state_parsing_string, state_parsing_label, state_parsing_variable,
			state_parsing_subscript, state_parsing_type_varlist, state_parsing_type_command,
			state_parsing_command_args, state_parsing_construct,
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
				unmatched_closing_delimiter,
				expected_identifier,			// expected an identifier (_id1/i_d1/_1id)
				expected_key_identifier,		// expected a key identifier (identifier/number/"string")
				expected_colon_punctuator,		// expected a :
				expected_integer_constant,
				expected_rhs_value,				// expected rhs of operation
				too_many_args,					// too many args, not enough params
				no_var_type_for_subexp,			// variable sub-expression begins with a type for which no compatible variable types were found

				// fatal errors
				fatal_begin = 4000,
				//include_failed = 4000,
				fatal_end,
			};

			Error(ID id) : m_ID(id)
			{ }
			inline operator ID() const { return m_ID; }

			static inline std::string Formatter(Error type) { return s_map[type]; }

		private:
			static std::map<ID, std::string> s_map;

			ID m_ID;
		};

		// Parameter
		struct Parameter {
			Operand operand;
			const Types::Value* value;

			Parameter(Operand op, const Types::Value* val) : operand(op), value(val)
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
		
		// Events
		struct event : public build_event {
			explicit event(const Engine& engine) : build_event(engine)
			{ }
		};
		using event_begin = event;
		using event_finish = event;
		using event_warning = event;
		template<Error::ID TID, typename... TArgs>
		struct event_error : public error_event_data<TArgs...> {
			event_error(const Engine& engine, TArgs... args) : error_event_data(Basic::Error(engine, TID), std::forward<TArgs>(args)...)
			{ LinkEvent<event_error>("event_error"); }
		};
		using error_too_many_args				= event_error<Error::too_many_args>;
		using error_unmatched_closing_delimiter = event_error<Error::unmatched_closing_delimiter, Delimiter>;
		using error_expected_identifier			= event_error<Error::expected_identifier>;
		using error_expected_key_identifier		= event_error<Error::expected_key_identifier>;
		using error_expected_colon_punctuator	= event_error<Error::expected_colon_punctuator>;
		using error_expected_integer_constant	= event_error<Error::expected_integer_constant>;
		using error_expected_rhs_value			= event_error<Error::expected_rhs_value>;
		using error_invalid_identifier			= event_error<Error::invalid_identifier, Scripts::Range>;
		using error_invalid_command				= event_error<Error::invalid_command>;
		using error_invalid_operator			= event_error<Error::invalid_operator, Scripts::Range>;
		using error_invalid_character			= event_error<Error::invalid_character, Character>;
		using error_no_var_type_for_subexp		= event_error<Error::no_var_type_for_subexp, const Types::Type*>;

		Types::Xlation FormArgumentXlate(const Types::Xlation&, const Tokens::CommandArgs::Arg&);

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

				VecRef<Types::Type> type;
				Tokens::Iterator type_iterator;
				IToken* token;
				std::vector<TypeVarDeclaration> var_declarations;

				TypeParseState()
				{ }
				TypeParseState(VecRef<Types::Type> type_, IToken* token_) : type(type_), token(token_)
				{ }
			} m_TypeParseState;
			struct CommandParseState {
				VecRef<Command> command;
				CommandArg::Iterator commandArgIt;
				std::vector<Tokens::CommandArgs::Arg> args;

				CommandParseState() = default;
				void Clear() {
					command = nullptr;
					args.clear();
				}
				void Begin(VecRef<Command> cmd, CommandArg::Iterator argit) {
					Clear();
					command = cmd;
					commandArgIt = argit;
				}
				Tokens::CommandArgs::Arg* AddArg(Operand op, Types::Value* value) {
					args.emplace_back(op, value);
					return &args.back();
				}
				inline size_t NumArgs() const { return args.size(); }
				inline bool AreParamsSatisfied() const { return command->NumRequiredArgs() <= NumArgs(); }
				inline bool AnyParamsLeft() const { return command->NumParams() > NumArgs(); }
				
			} m_CommandParseState;
			struct OperationParseState_ {
				struct ParseResult {
					VecRef<Types::Xlation> xlate;
					size_t size;
				};
				enum State {
					init, startOperator, startVariable, startValue,
					waitRHS, waitUnaryRHS, startValueWaitRHS, finishedRHS, finishedUnary, finishedUnaryRHS,
				} m_State = init;
				enum class OperandType {
					Operation, Operator, PreUnaryOperator, PostUnaryOperator, Value, Variable
				};

				Parser& m_Parser;
				bool m_SubEval = false;
				bool m_Chaining = false;
				bool m_Negate = false;
				int m_NumVars = 0;
				int m_NumVals = 0;
				std::vector<OperandType> m_OperandTypes;
				std::vector<std::pair<Operand, const SCRambl::Types::Type*>> m_OperandChain;
				std::vector<Operators::OperationRef> m_OperationChain;
				std::vector<Operators::OperatorRef> m_OperatorChain;
				Operators::OperatorRef m_Operator;
				ScriptVariable* m_EvalVar;

				OperationParseState_(Parser& parser, bool subeval = false) : m_Parser(parser), m_SubEval(subeval)
				{ Reset(); }
				
				void Reset() {
					m_State = init;
					m_Chaining = false;
					m_Negate = false;
					m_NumVars = 0;
					m_NumVals = 0;
					m_Operator = nullptr;
					m_EvalVar = nullptr;
					m_OperandChain.clear();
					m_OperationChain.clear();
				}
				void AddOperator(Operators::OperatorRef op) {
					m_OperandTypes.emplace_back(OperandType::Operator);
					m_OperatorChain.emplace_back(op);
				}
				Operators::OperatorRef PopOperator() {
					ASSERT(!m_OperatorChain.empty());
					
					auto op = m_OperatorChain.back();
					for (auto it = m_OperandTypes.rbegin(); it != m_OperandTypes.rend(); ++it) {
						if (*it == OperandType::Operator) {
							m_OperandTypes.erase(it.base());
							break;
						}
					}
					m_OperatorChain.pop_back();
					return op;
				}
				// return true if valid
				bool MeetOperator(Operators::OperatorRef oper) {
					if (m_State == init) {
						// await prefix op
						m_State = startOperator;
						m_Operator = oper;
						return true;
					}
					else if (m_State == startVariable || m_State == startValue) {
						// wait for RHS or complete postfix op
						m_State = m_State == startValue ? startValueWaitRHS : waitRHS;
						m_Operator = oper;
						return true;
					}
					else if (m_State == startOperator);
					else if (m_State == waitRHS || m_State == startValueWaitRHS || m_State == finishedUnaryRHS) {
						if (oper->IsNegative()) {
							m_Negate = !m_Negate;
							return true;
						}
						else {
							if (!m_SubEval) {
								// TODO: check for increment auto-operation?
								// try to complete postfix unary
								if (m_OperandTypes.back() == OperandType::Variable) {
									if (auto op = m_Operator->GetUnaryOperation(m_OperandChain.back().first.Value<ScriptVariable>().Ptr(), false)) {
										AddOperation(op);
										AddVariable(&m_OperandChain.back().first.Value<ScriptVariable>());
										m_State = finishedUnary;
									}
								}

								if (m_State != finishedUnary) {
									m_State = waitUnaryRHS;
									AddOperator(m_Operator);
								}
							}
							else {
								AddOperator(m_Operator);
							}

							// proceed with chaining
							m_Operator = oper;
							m_Chaining = true;
						}
						return true;
					}
					else if (m_State == finishedRHS) {
						// chain!
						m_Operator = oper;
						m_Chaining = true;
						m_State = waitRHS;
						return true;
					}
					return false;
				}
				void AddVariable(ScriptVariable* var) {
					m_OperandTypes.emplace_back(OperandType::Variable);
					m_OperandChain.emplace_back(m_Negate ? Operand(var).Negate() : var, var->Get().Type());
					++m_NumVars;
				}
				// return true if valid
				bool MeetVariable(ScriptVariable* var) {
					if (m_State == init) {
						// await postfix op or full op
						AddVariable(var);
						m_State = startVariable;
						return true;
					}
					else if (m_State == startOperator || m_State == waitUnaryRHS) {
						// complete prefix op
						auto op = m_Operator->GetUnaryOperation(var->Ptr(), true);
						if (op) {
							AddOperation(op);
							AddVariable(var);
							m_Operator = PopOperator();
							m_State = finishedUnaryRHS;
							return MeetVariable(var);		// weee'll meet again...
						}
					}
					else if (m_State == waitRHS || m_State == startValueWaitRHS || m_State == finishedUnaryRHS) {
						// complete op
						if (!m_SubEval) {
							auto op = m_State == startValueWaitRHS
								? m_Operator->GetOperation(var->Ptr(), var->Get().Type())
								: m_Operator->GetOperation(m_OperandChain.front().first.Value<ScriptVariable>().Ptr(), var->Get().Type());
							if (op) {
								AddOperation(op);
								AddVariable(var);
								m_State = finishedRHS;
								return true;
							}
						}
						else {
							AddOperator(m_Operator);
							AddVariable(var);
							m_State = finishedRHS;
							return true;
						}
					}
					return false;
				}
				void AddOperation(Operators::OperationRef op) {
					m_OperationChain.emplace_back(op);
					m_OperandTypes.emplace_back(OperandType::Operation);
				}
				void AddValue(Operand operand, const Types::Type* type) {
					if (m_Negate) operand.Negate();
					m_OperandChain.emplace_back(operand, type);
					m_OperandTypes.emplace_back(OperandType::Value);
					++m_NumVals;
				}
				// return true if valid
				bool MeetValue(Operand operand, const Types::ITypeRef& type) {
					return MeetValue(operand, type.Ptr());
				}
				bool MeetValue(Operand operand, const Types::Type* type) {
					if (operand.GetType() == Operand::VariableValue)
						return MeetVariable(&operand.Value<ScriptVariable>());
					if (m_State == init && m_SubEval) {
						AddValue(operand, type);
						m_State = startValue;
						return true;
					}
					if (m_State == waitRHS || m_State == finishedUnary || (m_State == startValueWaitRHS && m_SubEval)) {
						// if we're chaining or in sub-expression, check the chained operations for extra opportunities...
						if (m_Chaining || (m_SubEval && !m_OperandChain.empty())) {
							// allow for auto operations on consecutive constants
							if	(m_OperandChain.back().first.GetType() != Operand::VariableValue) {
								auto& last_operand = m_OperandChain.back();
								if (m_Parser.m_BuildConfig->Optimisation().CheckLevel(OptimisationConfig::CHAIN_CONST_OPS)) {
									if (m_Operator->HasAuto()) {
										auto op = m_Operator->GetAutoOperation(last_operand.second, type);
										if (op) {
											switch (op->AutoType()) {
											default:
												// if there have been variables we can't auto evaluate const expression operators, 'cept these...
												if (!m_NumVars) {
												case Operators::Type::add:
												case Operators::Type::sub:
												case Operators::Type::inc:
												case Operators::Type::dec:
												case Operators::Type::bit_not:
												case Operators::Type::eq:
												case Operators::Type::neq:
												case Operators::Type::lt:
												case Operators::Type::gt:
												case Operators::Type::leq:
												case Operators::Type::geq:
												case Operators::Type::not:
												case Operators::Type::and:
												case Operators::Type::or:
													// evaluate new operand
													if (m_Negate) operand.Negate();
													last_operand.first = op->EvaluateAuto(last_operand.first, operand);
													m_State = finishedRHS;
													return true;
												}
												break;
											}
										}
									}
								}
							}
						}

						if (!m_SubEval) {
							auto op = m_Operator->GetOperation(m_OperandChain.front().first.Value<ScriptVariable>().Ptr(), type);
							if (op) {
								AddOperation(op);
								AddValue(operand, type);
								m_State = finishedRHS;
								return true;
							}
						}
						else {
							AddOperator(m_Operator);
							AddValue(operand, type);
							m_State = finishedRHS;
							return true;
						}
					}
					return false;
				}
				// return true if successful
				bool Finish() {
					if (m_OperandChain.empty() || (m_OperationChain.empty() && !m_SubEval)) return false;

					// create var and assignment for sub-evaluations at start of operations where needed
					if (m_SubEval && (m_NumVars || m_NumVals > 1)) {
						// for this we'll just rebuild the operations...
						OperationParseState_ newState(m_Parser);
						m_EvalVar = m_Parser.CreateParseVar(m_OperandChain.begin()->second, 0);
						auto& defOperators = m_Parser.m_Build.GetOperators().DefaultOperators();
						auto& firstOp = m_OperandChain.begin();
						Operators::OperatorRef opr;
						Operators::OperatorRef defop;
						
						// try to find an operation for assignment of sub-expression
						for (auto& defOp : defOperators) {
							if (!defOp->IsAssignment()) continue;
							if (defOp->IsConditional()) continue;

							// almost there, is there a valid operation here?
							if (defOp->GetOperation(m_EvalVar->Ptr(), firstOp->second)) {
								defop = defOp;
								break;
							}
						}
						
						newState.MeetVariable(m_EvalVar);
						newState.MeetOperator(defop);

						// rebuild the rest of the operation
						size_t opidx = 0;
						size_t operidx = 0;
						size_t opertidx = 0;
						
						for (auto type : m_OperandTypes) {
							switch (type) {
							case OperandType::Operator:
								newState.MeetOperator(m_OperatorChain[opidx++]);
								break;
							case OperandType::Operation:
								BREAK();
								newState.AddOperation(m_OperationChain[opertidx++]);
								opr = nullptr;
								break;
							case OperandType::Value:
								newState.MeetValue(m_OperandChain[operidx].first, m_OperandChain[operidx].second);
								++operidx;
								break;
							case OperandType::Variable:
								newState.MeetValue(m_OperandChain[operidx].first, m_OperandChain[operidx].second);
								++operidx;
								break;
							}
						}

						if (!newState.Finish()) {
							BREAK();		// error
							return false;
						}
						return true;
					}

					if (m_State != finishedRHS && m_State != finishedUnary) return false;

					auto const begin_it = m_OperandChain.begin();
					auto operand_it = m_OperandChain.begin();
					for (auto& oper : m_OperationChain) {
						Operators::OperationValue* opval = nullptr;
						auto& op = oper->GetOperator();
						auto attributes = oper->GetAttributes();
						
						op->Type()->Values<Operators::OperationValue>(Types::ValueSet::Operation, [&attributes, &opval](Operators::OperationValue* value){
							if (value->CanFitSize(value->GetValueSize(attributes))) {
								opval = value;
								return true;
							}
							return false;
						});
						
						if (!opval) BREAK();

						auto& xlation = m_Parser.m_Xlation;
						xlation = m_Parser.m_Build.AddSymbol(opval->GetTranslation());
						xlation->SetAttributes(Types::DataSourceID::Command, attributes);

						size_t size = xlation->GetTranslation()->GetSize(*xlation);
						std::vector<Tokens::CommandArgs::Arg> args;

						// sort lhs
						if (oper->HasLHS() || oper->HasLHV()) {
							if (oper->HasLHV()) {
								Numbers::IntegerType v = static_cast<long long>(oper->GetLHV());
								auto value = m_Parser.GetBestValue(Types::ValueSet::Number, v.Size());
								if (!value) BREAK();
								args.emplace_back(Operand(v, m_Parser.m_Engine.Format(v)), value);
								size += value->GetTranslation()->GetSize(FormArgumentXlate(*xlation, args.back()));
							}
							else if (operand_it != m_OperandChain.end()) {
								auto& svar = (oper->HasRHV() ? operand_it : begin_it)->first.Value<ScriptVariable>();
								auto var = svar.Ptr();
								auto value = m_Parser.AllFittingValues<Types::VariableValue>(Types::ValueSet::Variable, var->Value()->GetSize(), [&var](Types::VariableValue* value){
									return value->IsGlobal() == var->IsGlobal();
								});
								if (!value) BREAK();
								args.emplace_back(Operand(&svar), value);
								size += value->GetTranslation()->GetSize(FormArgumentXlate(*xlation, args.back()));
								if (begin_it == operand_it) ++operand_it;
							}
							else BREAK();
						}
						if (oper->HasRHS() || oper->HasRHV()) {
							if (oper->HasRHV()) {
								Numbers::IntegerType v = static_cast<long long>(oper->GetRHV());
								auto value = m_Parser.GetBestValue(Types::ValueSet::Number, v.Size());
								if (!value) BREAK();
								
								if (oper->IsSwapped())
									args.emplace(args.begin(), Operand(v, m_Parser.m_Engine.Format(v)), value);
								else
									args.emplace_back(Operand(v, m_Parser.m_Engine.Format(v)), value);
								size += value->GetTranslation()->GetSize(FormArgumentXlate(*xlation, args.back()));
							}
							else if (operand_it != m_OperandChain.end()) {
								auto& operand = operand_it->first;
								if (operand.GetType() == Operand::VariableValue) {
									auto& svar = operand.Value<ScriptVariable>();
									auto var = svar.Ptr();
									auto value = m_Parser.AllFittingValues<Types::VariableValue>(Types::ValueSet::Variable, var->Value()->GetSize(), [&var](Types::VariableValue* value){
										return value->IsGlobal() == var->IsGlobal();
									});
									ASSERT(value);
									if (value) {
										if (oper->IsSwapped())
											args.emplace(args.begin(), operand, value);
										else
											args.emplace_back(operand, value);
										size += value->GetTranslation()->GetSize(FormArgumentXlate(*xlation, args.back()));
									}
									else {
										return false;
									}
								}
								else {
									Types::ValueSet valtype;
									if (operand.GetType() == Operand::Type::TextValue) {
										valtype = Types::ValueSet::Text;
									}
									else {
										valtype = Types::ValueSet::Number;
									}

									auto value = m_Parser.GetBestValue(valtype, operand.Size());
									ASSERT(value);
									if (value) {
										if (oper->IsSwapped())
											args.emplace(args.begin(), operand, value);
										else
											args.emplace_back(operand, value);
										size += value->GetTranslation()->GetSize(FormArgumentXlate(*xlation, args.back()));
									}
									else {
										return false;
									}
								}
							}
							else BREAK();

							++operand_it;
						}

						auto token = m_Parser.CreateToken<Tokens::CommandArgs::Info>(Tokens::Type::ArgList, args);
						m_Parser.m_SizeCount += BitsToBytes(size);
					}
					return true;
				}
			};
			std::vector<OperationParseState_> m_OperationParseStates;
			struct NumberParseState {
				bool ItIsFloat = false;
				bool ItIsNegated = false;
				union {
					Tokens::Number::Info<Numbers::IntegerType>* IntInfo = nullptr;
					Tokens::Number::Info<Numbers::FloatType>* FloatInfo;
				};
				NumberParseState() {
					IntInfo = nullptr;
				}
				void Start(Tokens::Number::Info<Numbers::IntegerType>* intInfo) {
					IntInfo = intInfo;
					ItIsFloat = false;
					ItIsNegated = false;
				}
				void Start(Tokens::Number::Info<Numbers::FloatType>* fltInfo) {
					FloatInfo = fltInfo;
					ItIsFloat = true;
					ItIsNegated = false;
				}
				void Negate() { ItIsNegated = true; }
				inline Numbers::IntegerType GetInt() const {
					auto v = Tokens::Number::GetNumberValue(*IntInfo);
					if (IsNegated()) v.Sign(true);
					return v;
				}
				inline Numbers::FloatType GetFloat() const {
					auto v = Tokens::Number::GetNumberValue(*FloatInfo);
					if (IsNegated()) v.Negate();
					return v;
				}
				inline bool IsNegated() const { return ItIsNegated; }
				inline bool IsFloat() const { return ItIsFloat; }
			} m_NumberParseState;
			struct ConstructParseState {
				enum State {
					init, parsing, parsingConditionList,
					finish
				};
				
				void Init(Constructing::Construct* construct_) {
					construct = construct_;
					if (construct->NumBlocks()) {
						blockStack.emplace(&construct->GetBlock(0));
						state = blockStack.top()->HasConditionList() ? parsingConditionList : parsing;
					}
					else state = finish;
				}

				void ProcessBlock() {
					auto block = blockStack.top();
					std::vector<Constructing::Data*> data;

					switch (state) {
					case parsing:
					case parsingConditionList:
						if (block->HasConditionList()) {
							if (state != parsingConditionList) {
								state = parsingConditionList;
							}
							break;
						}
						else state = init;

						// check if there is code data before the block
						if (block->GetDataAt(data, Constructing::DataPosition::Block, true)) {
							for (auto dat : data) {
								
							}
						}
						break;
					}
				}

				State state = init;
				Constructing::Construct* construct = nullptr;
				std::stack<Constructing::Block*> blockStack;
			} m_ConstructParseState;

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
			States Parse_Neutral_CheckLabel(IToken*);
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
			States Parse_Construct();

			inline bool IsEOLReached() const {
				return m_TokenIt == m_Tokens.end() || IsCharacterEOL(m_TokenIt->GetToken());
			}
			inline Tokens::Type GetCurrentTokenType() const {
				return m_TokenIt->GetToken()->GetType<Tokens::Type>();
			}
			static Types::NumberValueType GetNumberValueType(Types::Value* value) {
				return value->Extend<Types::NumberValue>().GetNumberType();
			}
			static bool IsNumberValueType(Types::Value* value, Types::NumberValueType type) {
				return GetNumberValueType(value) == type;
			}
			template<typename T = Types::Value, typename TFunc>
			inline T* AllFittingValues(Types::ValueSet type, size_t size, TFunc func) {
				T* best_value = nullptr;
				m_Build.GetTypes().AllValues(type, [&](Types::Value* value){
					if (value->CanFitSize(size) && (type != Types::ValueSet::Number || IsNumberValueType(value, Types::NumberValueType::Float) == m_NumberParseState.IsFloat())) {
						if (!best_value || best_value->GetSize() > value->GetSize()) {
							auto v = static_cast<T*>(value);
							if (func(v)) best_value = v;
						}
					}
					return false;
				});
				return best_value;
			}
			inline Types::Value* GetBestValue(Types::ValueSet type, size_t size) {
				Types::Value* best_value = nullptr;
				m_Build.GetTypes().AllValues(type, [this, type, size, &best_value](Types::Value* value){
					if (value->CanFitSize(size) && (type != Types::ValueSet::Number || IsNumberValueType(value, Types::NumberValueType::Float) == m_NumberParseState.IsFloat())) {
						if (!best_value || best_value->GetSize() > value->GetSize())
							best_value = value;
					}
					return false;
				});
				return best_value;
			}
			inline Types::Value* GetBestVarValue(ScriptVariable* variable) {
				auto size = CountBitOccupation((*m_Variable)->Index());
				auto& var = *variable;
				auto value = AllFittingValues<Types::VariableValue>(Types::ValueSet::Variable, size, [&var](Types::VariableValue* value){
					return value->IsGlobal() == var->IsGlobal();
				});
				return value;
			}

			IToken* PeekToken(Tokens::Type type = Tokens::Type::None, size_t off = 1);

			// Sends errors and returns default if fail
			template<typename T>
			T GetIntegerConstant(IToken* toke, T default_val = 0) {
				auto intinfo = GetIntInfo(toke);
				if (!intinfo) {
					m_Task.Event<error_expected_integer_constant>();
				}
				else {
					return static_cast<T>(*intinfo->GetValue<SCRambl::Tokens::Number::NumberValue>());
				}
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
				return Tokens::String::GetString(*toke);
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
				return IsTokenType(toke, Tokens::Type::Character);
			}
			static bool IsCharacterEOL(IToken* toke) {
				return IsCharacter(toke) && GetCharacterValue(toke) == Character::EOL;
			}
			static bool IsDelimiter(IToken* toke) {
				return IsTokenType(toke, Tokens::Type::Delimiter);
			}
			static bool IsSubscriptDelimiter(IToken* toke) {
				return IsDelimiter(toke) && Tokens::Delimiter::GetDelimiterType<Delimiter>(*toke) == Delimiter::Subscript;
			}
			static bool IsSubscriptDelimiterClosing(IToken* toke) {
				return IsSubscriptDelimiter(toke) && Tokens::Delimiter::GetScriptRange(*toke).End() == Tokens::Delimiter::GetScriptPosition(*toke);
			}
			static bool IsScopeDelimiter(IToken* toke) {
				return IsDelimiter(toke) && Tokens::Delimiter::GetDelimiterType<Delimiter>(*toke) == Delimiter::Scope;
			}
			static bool IsScopeDelimiterClosing(IToken* toke) {
				return IsScopeDelimiter(toke) && Tokens::Delimiter::GetScriptRange(*toke).End() == Tokens::Delimiter::GetScriptPosition(*toke);
			}
			static bool IsEvaluationDelimiter(IToken* toke) {
				return IsDelimiter(toke) && Tokens::Delimiter::GetDelimiterType<Delimiter>(*toke) == Delimiter::Evaluation;
			}
			static bool IsEvaluationDelimiterClosing(IToken* toke) {
				return IsEvaluationDelimiter(toke) && Tokens::Delimiter::GetScriptRange(*toke).End() == Tokens::Delimiter::GetScriptPosition(*toke);
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

			Tokens::CommandArgs::Arg* AddCommandArg(Operand, Types::Value*);
			bool GetDelimitedArrayIntegerConstant(size_t&);

			ScriptVariable* CreateParseVar(const Types::Type*, size_t array_size = 0);
			void FreeParseVar(ScriptVariable*);

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
			template<typename TTokenType, typename... TArgs>
			TTokenType* CreateToken(TArgs&&... args) {
				return m_Build.ParseToken<TTokenType>(Scripts::Range(m_TokenIt->GetPosition(), m_TokenIt->GetPosition()), args...)->GetToken<TTokenType>();
			}
			
		private:
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
			void AddLabel(const ScriptLabel* label, Tokens::Iterator it) {
				m_LabelReferences.emplace(label, LabelRef(it, false));
			}
			void AddLabelRef(const ScriptLabel* label, Tokens::Iterator it) {
				auto iter = m_LabelReferences.find(label);
				if (iter == m_LabelReferences.end()) {
					m_LabelReferences.emplace(label, LabelRef(it, true));
				}
				else {
					++iter->second.NumUses;
				}
			}

			inline VecRef<Types::Type> GetType(const std::string& name) {
				auto ptr = m_Types.GetType(name);
				return (ptr ? ptr : m_Build.GetTypes().GetType(name)).Ref();
			}

			void Init();
			void Finish();
			void Parse();

		private:
			State m_State = init;
			States m_ActiveState = state_neutral;
			States m_ParseState = state_neutral;
			Engine& m_Engine;
			Task& m_Task;
			Build& m_Build;
			BuildConfig* m_BuildConfig;
			Tokens::Storage& m_Tokens;
			Tokens::Iterator m_TokenIt;
			Tokens::Iterator m_LabelTokenIt;
			Tokens::Iterator m_VariableTokenIt;
			Tokens::Iterator m_CommandTokenIt;
			Tokens::Iterator m_OperatorTokenIt;
			std::vector<const Tokens::Iterator> m_CommandTokens;			// positions of all parsed command tokens
			std::vector<const Tokens::Iterator> m_LabelTokens;
			Types::Types& m_Types;
			size_t m_SizeCount;
			size_t m_NumCommandArgs;
			size_t m_NumOverloadFailures;
			ScriptVariable* m_Variable = nullptr;
			ScriptLabel* m_Label = nullptr;
			Commands& m_Commands;
			Commands m_ExtraCommands;
			VecRef<Command> m_CurrentCommand;
			Operators::OperatorRef m_CurrentOperator;
			Operators::OperatorType m_OperatorType;
			Command::Arg::Iterator m_CommandArgIt;
			Commands::Vector m_OverloadCommands;
			Commands::Vector::iterator m_OverloadCommandsIt;
			VecRef<Types::Xlation> m_Xlation;

			std::map<const ScriptLabel*, LabelRef> m_LabelReferences;
			std::vector<VecRef<Command>> m_CommandVector;
			std::unordered_map<std::string, size_t> m_CommandMap;
			std::multimap<const std::string, Tokens::Iterator> m_CommandTokenMap;
			std::vector<IToken*> m_Subscripts;
			std::vector<ScriptVariable*> m_ParseVars;
			std::set<ScriptVariable*> m_UsedParseVars;

			std::vector<Tokens::Token*> m_CommandArgTokens;

			// Status
			bool m_OnNewLine;
			bool m_ParsingCommandArgs;
			bool m_EndOfCommandArgs;
			bool m_Conditional;
		};
		// The parser task
		class Task : public TaskSystem::Task, private Parser {
			friend Parser;

		public:
			Task(Engine&, Build*);

			size_t GetProgressCurrent() const;
			size_t GetProgressTotal() const;
			Tokens::Token GetToken() const;

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
			Engine&	m_Engine;
		};
	}
}