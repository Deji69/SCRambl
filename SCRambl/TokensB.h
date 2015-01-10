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
			static const enum Parameter { TokenType, ScriptRange };
			template<typename TType = Type, typename ...TData>
			using Info = TokenInfo < TType, Script::Range, TData... >;
		};
		class Label {
		public:
			static const enum Parameter { ScriptRange, LabelValue };
			using Info = TokenInfo < Type, Script::Range, Script::Label::Shared >;
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
				Script::Range			m_Range;

			public:
				using Shared = std::shared_ptr < ValueToken >;

				Value(Type type, const TValue & value_type, Numbers::Type num_type, Script::Range rg) : ValueToken(type, value_type),
					m_Type(num_type), m_Range(rg)
				{ }
				Value(Type type, const TValue & value_type, const Info<Numbers::IntegerType>& info) : Value(type, value_type, info.GetValue<ValueType>(), info.GetValue<ScriptRange>()) {
					m_IntegerValue = info.GetValue<NumberValue>();
				}
				Value(Type type, const TValue & value_type, const Info<Numbers::FloatType>& info) : Value(type, value_type, info.GetValue<ValueType>(), info.GetValue<ScriptRange>()) {
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
			using Info = TokenInfo < Type, Script::Range, TCommandType >;
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
	}
}