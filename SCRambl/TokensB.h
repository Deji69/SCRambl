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
#include "Labels.h"
#include "Symbols.h"

namespace SCRambl
{
	/*\
	 * Tokens
	\*/
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
			using Shared = std::shared_ptr < ValueToken >;

			ValueToken(Tokens::Type tokentype, TValueType valuetype) : Symbol(tokentype), m_ValueType(valuetype)
			{ }
			inline virtual ~ValueToken()
			{ }

			TValueType GetValueType() const { return m_ValueType; }
			virtual size_t GetSize() const = 0;

		private:
			TValueType m_ValueType;
		};

		/*\
	 	 * Define token classes for each type
		\*/
		class None {
		public:
			static const enum Parameter { TokenType };
			using Info = TokenInfo < Type >;
		};
		class Directive {
		public:
			static const enum Parameter { TokenType, ScriptRange };
			using Info = TokenInfo<Type, Scripts::Range>;
		};
		class Identifier {
		public:
			static const enum Parameter { ScriptRange, EXTRA };
			static const size_t c_Extra = EXTRA;
			template<typename ...TData>
			using Info = TokenInfo<Type, Scripts::Range, TData...>;
		};
		class Label {
		public:
			static const enum Parameter { ScriptRange, LabelValue };
			using Info = TokenInfo<Type, Scripts::Range, Scripts::Label::Shared>;
			//static const enum RefParameter { ScriptRange, LabelValue };
			//using RefInfo = TokenInfo < Type, Scripts::Range, Scripts::Label::Shared >;

			/*\
			 * Tokens::Command::Call - Carries symbolic data for a command call
			\*/
			template<typename TLabelType>
			class Jump : public Symbol
			{
				Shared<TLabelType>				m_Label;
				size_t							m_Offset;

			public:
				Jump(std::shared_ptr<const Info> info, size_t off = 0) : Symbol(Type::CommandCall),
					m_Label(info->GetValue<LabelValue>()),
					m_Offset(off)
				{ }
			};
		};
		class Number {
		public:
			static const enum Parameter { ScriptRange, ValueType, NumberValue };
			template<typename TNumberType>
			using Info = TokenInfo<Type, Scripts::Range, Numbers::Type, TNumberType>;
			using TypelessInfo = TokenInfo<Type, Scripts::Range, Numbers::Type>;

			static Numbers::Type GetValueType(const IToken& token) {
				return token.Get<const TypelessInfo>().GetValue<ValueType>();
			}

			/*\
			 * Preprocessor::Number::Value - Carries all symbolic data for a number value
			\*/
			template<typename TValueType>
			class Value : public ValueToken<TValueType>
			{
				Numbers::Type			m_Type;
				Numbers::IntegerType	m_IntegerValue;
				Numbers::FloatType		m_FloatValue;

			public:
				using Shared = std::shared_ptr<ValueToken>;

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
		};
		class Operator {
		public:
			static const enum Parameter { ScriptRange, OperatorType };
			template<typename TOperatorType>
			using Info = TokenInfo < Type, Scripts::Range, TOperatorType >;
		};
		class Command {
		public:
			static const enum Parameter { ScriptRange, CommandType };
			template<typename TCommandType>
			using Info = TokenInfo < Type, Scripts::Range, Shared<TCommandType> >;
			template<typename TCommandType, typename TCont = std::vector<Shared<TCommandType>>>
			using OverloadInfo = TokenInfo < Type, Scripts::Range, TCont > ;

			/*\
			 * Tokens::Command:Decl - Carries symbolic data for a command declaration
			 \*/
			template<typename TCommandType>
			class Decl : public Symbol {
				size_t						m_ID;
				Shared<const TCommandType>	m_Command;

			public:
				Decl(size_t id, Shared<const TCommandType> ptr) : Symbol(Type::CommandDecl),
					m_ID(id), m_Command(ptr)
				{ }

				inline size_t GetID() const									{ return m_ID; }
				inline Shared<const TCommandType>& GetCommand()	const		{ return m_Command; }
			};

			/*\
			 * Tokens::Command::Call - Carries symbolic data for a command call
			\*/
			template<typename TCommandType>
			class Call : public Symbol
			{
				Shared<const TCommandType> m_Command;
				size_t m_NumArgs;
				std::vector<Shared<Symbol>> m_Args;

			public:
				Call(const Info<TCommandType>& info, size_t num_args) : Symbol(Type::CommandCall),
					m_Command(info.GetValue<CommandType>()),
					m_NumArgs(num_args)
				{
					//Command::
				}

				inline Shared<const TCommandType> GetCommand() const	{ return m_Command; }
				inline size_t GetNumArgs() const						{ return m_NumArgs; }

				inline void AddArg(Shared<Symbol> symbol) { m_Args.emplace_back(symbol); }
			};
		};
		class String {
		public:
			static const enum Parameter { ScriptRange, StringValue };
			using Info = TokenInfo < Type, Scripts::Range, std::string >;

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
		};
		class Delimiter {
		public:
			static const enum Parameter { ScriptPosition, ScriptRange, DelimiterType };
			template<typename TDelimiterType>
			using Info = TokenInfo < Type, Scripts::Position, Scripts::Range, TDelimiterType >;
		};
		class Character {
		public:
			static const enum Parameter { ScriptPosition, CharacterValue };
			template<typename TCharType>
			using Info = TokenInfo<Type, Scripts::Position, TCharType>;
		};

		template<typename TTokenType, typename... TArgs>
		static Shared<TTokenType> CreateToken(TArgs&&... args) {
			return std::make_shared<TTokenType>(args...);
		}
		template<typename T>
		static T& GetToken(IToken::Shared token) {
			return token->Get<T>();
		}
	}
}