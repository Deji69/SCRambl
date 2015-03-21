//**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Scripts.h"
#include "Tokens.h"
#include "Numbers.h"

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
		template<typename TValue>
		class ValueToken : public Symbol
		{
		public:
			using Shared = std::shared_ptr < ValueToken >;

			ValueToken(Tokens::Type type, const TValue & value_type) : Symbol(type), m_ValueType(value_type)
			{ }
			inline virtual ~ValueToken()
			{ }

		private:
			const TValue &		m_ValueType;
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
			using Info = TokenInfo < Type, Script::Range >;
		};
		class Identifier {
		public:
			static const enum Parameter { TokenType, ScriptRange, EXTRA };
			static const size_t c_Extra = EXTRA;
			template<typename TType = Type, typename ...TData>
			using Info = TokenInfo < TType, Script::Range, TData... >;
		};
		class Label {
		public:
			static const enum Parameter { ScriptRange, LabelValue };
			using Info = TokenInfo < Type, Script::Range, Script::Label::Shared >;
			//static const enum RefParameter { ScriptRange, LabelValue };
			//using RefInfo = TokenInfo < Type, Script::Range, Script::Label::Shared >;

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
			using Info = TokenInfo < Type, Script::Range, Numbers::Type, TNumberType >;
			using TypelessInfo = TokenInfo < Type, Script::Range, Numbers::Type >;

			static Numbers::Type GetValueType(const IToken& token) {
				return token.Get<const TypelessInfo>().GetValue<ValueType>();
			}

			/*\
			 * Preprocessor::Number::Value - Carries all symbolic data for a number value
			\*/
			template<typename TValue>
			class Value : public ValueToken<TValue>
			{
				Numbers::Type			m_Type;
				Numbers::IntegerType	m_IntegerValue;
				Numbers::FloatType		m_FloatValue;

			public:
				using Shared = std::shared_ptr < ValueToken >;

				Value(Type type, const TValue & value_type, Numbers::Type num_type) : ValueToken(type, value_type),
					m_Type(num_type)
				{ }
				Value(Type type, const TValue & value_type, const Info<Numbers::IntegerType>& info) : Value(type, value_type, info.GetValue<ValueType>()) {
					m_IntegerValue = info.GetValue<NumberValue>();
				}
				Value(Type type, const TValue & value_type, const Info<Numbers::FloatType>& info) : Value(type, value_type, info.GetValue<ValueType>()) {
					m_FloatValue = info.GetValue<NumberValue>();
				}
			};
		};
		class Operator {
		public:
			static const enum Parameter { ScriptRange, OperatorType };
			template<typename TOperatorType>
			using Info = TokenInfo < Type, Script::Range, TOperatorType >;
		};
		class Command {
		public:
			static const enum Parameter { ScriptRange, CommandType };
			template<typename TCommandType>
			using Info = TokenInfo < Type, Script::Range, Shared<TCommandType> >;
			template<typename TCommandType, typename TCont = std::vector<Shared<TCommandType>>>
			using OverloadInfo = TokenInfo < Type, Script::Range, TCont > ;

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
				Shared<const TCommandType>		m_Command;
				size_t							m_NumArgs;

			public:
				Call(const Info<TCommandType>& info, size_t num_args) : Symbol(Type::CommandCall),
					m_Command(info.GetValue<CommandType>()),
					m_NumArgs(num_args)
				{
					//Command::
				}

				inline Shared<const TCommandType> GetCommand() const	{ return m_Command; }
				inline size_t GetNumArgs() const						{ return m_NumArgs; }
			};
		};
		class String {
		public:
			static const enum Parameter { ScriptRange, StringValue };
			using Info = TokenInfo < Type, Script::Range, std::string >;
		};
		class Character {
		public:
			static const enum Parameter { ScriptPosition, CharacterValue };
			template<typename TCharacterType>
			using Info = TokenInfo < Type, Script::Position, TCharacterType >;
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