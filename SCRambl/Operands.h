/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Numbers.h"
#include "Tokens.h"
#include "Types.h"
#include "Builder.h"

namespace SCRambl {
	enum class NumberAttributeID {
		None, Value, Size,
	};
	enum class LabelAttributeID {
		None, Offset, Name,
	};
	enum class VariableAttributeID {
		None, Index, Name, Size, IsArray, IsGlobal
	};
	enum class TextAttributeID {
		None, Value, Size,
	};

	class NumberAttributeSet : public AttributeSet<NumberAttributeID> {
	public:
		NumberAttributeSet() : AttributeSet(NumberAttributeID::None) {
			AddAttribute("Value", NumberAttributeID::Value);
			AddAttribute("Size", NumberAttributeID::Size);
		}
	};
	class TextAttributeSet : public AttributeSet<TextAttributeID> {
	public:
		TextAttributeSet() : AttributeSet(TextAttributeID::None) {
			AddAttribute("Value", TextAttributeID::Value);
			AddAttribute("Size", TextAttributeID::Size);
		}
	};
	class LabelAttributeSet : public AttributeSet<LabelAttributeID> {
	public:
		LabelAttributeSet() : AttributeSet(LabelAttributeID::None) {
			AddAttribute("Offset", LabelAttributeID::Offset);
			AddAttribute("Name", LabelAttributeID::Name);
		}
	};
	class VariableAttributeSet : public AttributeSet<VariableAttributeID> {
	public:
		VariableAttributeSet() : AttributeSet(VariableAttributeID::None) {
			AddAttribute("Index", VariableAttributeID::Index);
			AddAttribute("Name", VariableAttributeID::Name);
			AddAttribute("Size", VariableAttributeID::Size);
		}
	};

	class NumberAttributes : public Attributes<NumberAttributeID, NumberAttributeSet> {
	public:
		NumberAttributes()
		{ }
	};
	class TextAttributes : public Attributes<TextAttributeID, TextAttributeSet> {
	public:
		TextAttributes()
		{ }
	};
	class LabelAttributes : public Attributes<LabelAttributeID, LabelAttributeSet> {
	public:
		LabelAttributes()
		{ }
	};
	class VariableAttributes : public Attributes<VariableAttributeID, VariableAttributeSet> {
	public:
		VariableAttributes()
		{ }
	};

	class Operand {
		friend class NumberAttributes;
		friend class TextAttributes;
		friend class LabelAttributes;
		friend class VariableAttributes;

	public:
		using Attributes = Attributes<Types::DataAttributeID, Types::DataAttributeSet>;
		enum Type { NullValue, IntValue, FloatValue, TextValue, LabelValue, VariableValue };

		Operand() = default;
		Operand(Variable* var) : m_Type(VariableValue),
			m_VariableValue(var), m_Text(var->Name())
		{ }
		Operand(Label* label) : m_Type(LabelValue),
			m_LabelValue(label), m_Text(label->Name())
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
			m_IntValue(0), m_FloatValue(v), m_Text(str)
		{
		}
		Operand(std::string v) : m_Type(TextValue),
			m_Text(v)
		{ }

		template<typename T>
		inline T Value() const { return m_IntValue; }
		template<> inline long long Value() const { return m_IntValue; }
		template<> inline float Value() const { return m_FloatValue; }
		template<> inline Label* Value() const { return m_LabelValue; }
		template<> inline Variable* Value() const { return m_VariableValue; }
		inline Type GetType() const { return m_Type; }
		inline std::string Text() const { return m_Text; }

		Attributes GetNumberAttributes() const;
		Attributes GetTextAttributes() const;
		Attributes GetLabelAttributes() const;
		Attributes GetVariableAttributes() const;

	private:
		Type m_Type = NullValue;
		union {
			int64_t m_IntValue = 0;
			float m_FloatValue;
			Label* m_LabelValue;
			Variable* m_VariableValue;
		};
		std::string m_Text;
	};
}