/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Numbers.h"
#include "Tokens.h"

namespace SCRambl {
	class Operand {
	public:
		enum Type { NullValue, IntValue, FloatValue, TextValue, LabelValue, VariableValue };

		Operand() = default;
		Operand(ScriptVariable* var) : m_Type(VariableValue),
			m_VariableValue(var), m_Text(var->Get().Name())
		{ }
		Operand(ScriptLabel* label) : m_Type(LabelValue),
			m_LabelValue(label), m_Text(label->Get().Name())
		{ }
		Operand(Tokens::Number::Info<Numbers::IntegerType>* info) : m_Type(IntValue),
			m_IntValue(*info->GetValue<Tokens::Number::NumberValue>()),
			m_Text(info->GetValue<Tokens::Number::ScriptRange>().Format())
		{ }
		Operand(Tokens::Number::Info<Numbers::FloatType>* info) : m_Type(FloatValue),
			m_FloatValue(*info->GetValue<Tokens::Number::NumberValue>()),
			m_Text(info->GetValue<Tokens::Number::ScriptRange>().Format())
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

		template<typename T>
		inline T Value() const;
		template<> inline int64_t Value() const { return m_IntValue; }
		template<> inline float Value() const { return m_FloatValue; }
		template<> inline ScriptLabel* Value() const { return m_LabelValue; }
		template<> inline ScriptVariable* Value() const { return m_VariableValue; }
		inline Type GetType() const { return m_Type; }
		inline std::string Text() const { return m_Text; }

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
}