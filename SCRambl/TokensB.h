//**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Scripts-code.h"
#include "Tokens.h"
#include "Numbers.h"
//#include "Labels.h"
//#include "Commands.h"
//#include "Variables.h"
#include "Symbols.h"
#include "ScriptObjects.h"

namespace SCRambl
{
	class Command;
	class Label;
	class Variable;

	/*\ Tokens \*/
	namespace Tokens
	{
		/*\
		 * Tokens::ValueToken
		 * TValue - Value type
		\*/
		template<typename TValueType>
		class ValueToken : public Symbol
		{
		public:
			ValueToken(Tokens::Type tokentype, TValueType valuetype) : Symbol(tokentype), m_ValueType(valuetype)
			{ }
			inline virtual ~ValueToken()
			{ }

			TValueType GetValueType() const { return m_ValueType; }
			virtual size_t GetSize() const = 0;

		private:
			TValueType m_ValueType;
		};

		/*\ Define token classes for each type \*/
		class None {
		public:
			static const enum Parameter { };
			using Info = TokenInfo<Type>;
		};
		class Directive {
		public:
			static const enum Parameter { ScriptRange };
			using Info = TokenInfo<Type, Scripts::Range>;

			static Scripts::Range GetScriptRange(const IToken& token) {
				return token.Get<Info>().GetValue<ScriptRange>();
			}
		};
		class Identifier {
		public:
			static const enum Parameter { ScriptRange, EXTRA };
			static const size_t c_Extra = EXTRA;
			template<typename... TData>
			using Info = TokenInfo<Type, Scripts::Range, TData...>;

			static Scripts::Range GetScriptRange(const IToken& token) {
				return token.Get<const Info<>>().GetValue<ScriptRange>();
			}
		};
		class Label {
		public:
			static const enum Parameter { ScriptRange, LabelValue };
			using Info = TokenInfo<Type, Scripts::Range, SCRambl::Label*>;

			static Scripts::Range GetScriptRange(const IToken& token) {
				return token.Get<Info>().GetValue<ScriptRange>();
			}
			static SCRambl::Label* GetLabel(const IToken& token) {
				return token.Get<Info>().GetValue<LabelValue>();
			}

			/* Tokens::Command::Call - Carries symbolic data for a command call */
			class Jump : public Symbol
			{
				const SCRambl::Label* m_Label;
				size_t m_Offset;

			public:
				Jump(const Info* info, size_t off = 0) : Symbol(Type::CommandCall),
					m_Label(info->GetValue<LabelValue>()),
					m_Offset(off)
				{ }
			};
		};
		class Number {
		public:
			static const enum Parameter { ScriptRange, ValueType, NumberValue };
			template<typename TNumberType>
			using Info = TokenInfo<Type, Scripts::Range, Numbers::Type, TNumberType*>;
			using DummyInfo = TokenInfo<Type, Scripts::Range, Numbers::Type, void*>;

			static Scripts::Range GetScriptRange(const IToken* token) {
				return token->Get<DummyInfo>().GetValue<ScriptRange>();
			}
			static Numbers::Type GetValueType(const IToken& token) {
				return token.Get<DummyInfo>().GetValue<ValueType>();
			}
			static bool IsTypeInt(const IToken& token) {
				switch (token.Get<DummyInfo>().GetValue<ValueType>()) {
				case Numbers::Type::Integer:
				case Numbers::Type::DWord:
				case Numbers::Type::Word:
				case Numbers::Type::Byte:
					return true;
				}
				return false;
			}
			static bool IsTypeFloat(const IToken& token) {
				return token.Get<DummyInfo>().GetValue<ValueType>() == Numbers::Type::Float;
			}
			template<typename TNumberType>
			static TNumberType GetNumberValue(const IToken& token) {
				return token.Get<Info<TNumberType>>().GetValue<NumberValue>();
			}

			/*\ Tokens::Number::Value - Carries all symbolic data for a number value \*/
			template<typename TValueType>
			class Value : public ValueToken<TValueType>
			{
				Numbers::Type m_Type;
				Numbers::IntegerType m_IntegerValue;
				Numbers::FloatType m_FloatValue;

			public:
				Value(TValueType valtype, Numbers::Type num_type) : ValueToken(Type::Number, valtype),
					m_Type(num_type)
				{ }
				Value(TValueType valtype, const Info<Numbers::IntegerType>& info) : Value(valtype, info.GetValue<ValueType>()) {
					m_IntegerValue = info.GetValue<NumberValue>();
				}
				Value(TValueType valtype, const Info<Numbers::FloatType>& info) : Value(valtype, info.GetValue<ValueType>()) {
					m_FloatValue = info.GetValue<NumberValue>();
				}

				virtual size_t GetSize() const override {
					switch (m_Type) {
					case Numbers::Type::Float: return 4;
					case Numbers::Type::Byte: return 1;
					case Numbers::Type::Word: return 2;
					case Numbers::Type::Integer: return 4;
					case Numbers::Type::DWord: return 4;
					}
					BREAK();
					return 0;
				}
			};
		
			/*\ Tokens::Number::Symbol \*/
			class SymbolInfo : public Symbol {
			public:
				SymbolInfo() : Symbol(Tokens::Type::Number)
				{

				}
			};
		};
		class Operator {
		public:
			enum Parameter { ScriptRange, OperatorType };
			template<typename TOperatorType>
			using Info = TokenInfo<Type, Scripts::Range, TOperatorType>;
			using DummyInfo = TokenInfo<Type, Scripts::Range>;

			template<typename TOperationType>
			class Operation : public Symbol
			{
			public:
				Operation(TOperationType* op) : Symbol(Type::Operator),
					m_Operation(op)
				{ }

				TOperationType* GetOperation() const { return m_Operation; }

			private:
				TOperationType* m_Operation;
			};
		
			static Scripts::Range GetScriptRange(const IToken& token) {
				return token.Get<DummyInfo>().GetValue<ScriptRange>();
			}
			template<typename TOperatorType>
			static TOperatorType GetOperator(const IToken& token) {
				return token.Get<Info<TOperatorType>>().GetValue<OperatorType>();
			}
		};
		class Command {
		public:
			enum Parameter { ScriptRange, CommandType };
			using Info = TokenInfo<Type, Scripts::Range, SCRambl::Command*>;
			using OverloadInfo = TokenInfo<Type, Scripts::Range, std::vector<SCRambl::Command*>>;
			using DummyInfo = TokenInfo<Type, Scripts::Range>;

			/*\ Tokens::Command:Decl - Carries symbolic data for a command declaration \*/
			class Decl : public Symbol {
				size_t m_ID;
				const SCRambl::Command* m_Command;

			public:
				Decl(size_t id, const SCRambl::Command* ptr) : Symbol(Type::CommandDecl),
					m_ID(id), m_Command(ptr)
				{ }

				inline size_t GetID() const { return m_ID; }
				inline const SCRambl::Command* GetCommand()	const { return m_Command; }
			};

			/*\ Tokens::Command::Call - Carries symbolic data for a command call \*/
			class Call : public Symbol {
				const SCRambl::Command* m_Command;
				size_t m_NumArgs;
				std::vector<Symbol*> m_Args;

			public:
				Call(const Info& info, size_t num_args) : Symbol(Type::CommandCall),
					m_Command(info.GetValue<CommandType>()),
					m_NumArgs(num_args)
				{ }
				Call(const IToken* info, size_t num_args) : Call(info->Get<Info>(), num_args)
				{ }

				inline const SCRambl::Command* GetCommand() const { return m_Command; }
				inline size_t GetNumArgs() const { return m_NumArgs; }

				inline void AddArg(Symbol* symbol) { m_Args.emplace_back(symbol); }
			};

			static Scripts::Range GetScriptRange(const IToken& token) {
				return token.Get<DummyInfo>().GetValue<ScriptRange>();
			}
			static SCRambl::Command* GetCommand(const IToken& token) {
				return token.Get<Info>().GetValue<CommandType>();
			}
			static std::vector<SCRambl::Command*> GetOverloads(const IToken& token) {
				return token.Get<OverloadInfo>().GetValue<CommandType>();
			}
		};
		class String {
		public:
			enum Parameter { ScriptRange, StringValue };
			using Info = TokenInfo<Type, Scripts::Range, std::string>;

			template<typename TValueType>
			class Value : public ValueToken<TValueType>
			{
				std::string m_Value;

			public:
				Value(TValueType value, std::string val) : ValueToken(Type::String, value), m_Value(val)
				{ }
				Value(TValueType value, const Info& info) : Value(info.GetValue<StringValue>())
				{ }

				const std::string& GetValue() const { return m_Value; }
				virtual size_t GetSize() const { return m_Value.size(); }
			};
			class Object : public Symbol
			{
				std::string m_Value;

			public:
				Object(std::string val) : Symbol(Type::String), m_Value(val)
				{ }

				std::string Value() const { return m_Value; }
				size_t Size() const { return Value().size(); }
			};
		
			static Scripts::Range GetScriptRange(const IToken& token) {
				return token.Get<Info>().GetValue<ScriptRange>();
			}
			static std::string GetString(const IToken& token) {
				return token.Get<Info>().GetValue<StringValue>();
			}
		};
		class Delimiter {
		public:
			enum Parameter { ScriptPosition, ScriptRange, DelimiterType };
			template<typename TDelimiterType>
			using Info = TokenInfo<Type, Scripts::Position, Scripts::Range, TDelimiterType>;
			using DummyInfo = TokenInfo<Type, Scripts::Position, Scripts::Range>;

			static Scripts::Position GetScriptPosition(const IToken& token) {
				return token.Get<DummyInfo>().GetValue<ScriptPosition>();
			}
			static Scripts::Range GetScriptRange(const IToken& token) {
				return token.Get<DummyInfo>().GetValue<ScriptRange>();
			}
			template<typename TDelimiterType>
			static TDelimiterType GetDelimiterType(const IToken& token) {
				return token.Get<Info<TDelimiterType>>().GetValue<DelimiterType>();
			}
		};
		class Character {
		public:
			enum Parameter { ScriptPosition, CharacterValue };
			template<typename TCharType>
			using Info = TokenInfo<Type, Scripts::Position, TCharType>;
			using TypelessInfo = TokenInfo<Type, Scripts::Position>;

			template<typename TCharType>
			static TCharType GetCharacter(const IToken& token) {
				return token.Get<const Info<TCharType>>().GetValue<CharacterValue>();
			}
			static Scripts::Position GetPosition(const IToken& token) {
				return token.Get<const TypelessInfo>().GetValue<ScriptPosition>();
			}
		};
	}
}