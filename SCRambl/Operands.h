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
		Operand(ScriptVariable* var) : m_Type(VariableValue),
			m_VariableValue(var), m_Text((*var)->Name())
		{ }
		Operand(ScriptLabel* label) : m_Type(LabelValue),
			m_LabelValue(label), m_Text((*label)->Name())
		{ }
		Operand(int64_t v, std::string str) : m_Type(IntValue),
			m_UIntValue(v), m_Text(str), m_Size(CountBitOccupation(v))
		{ }
		Operand(double v, std::string str) : m_Type(FloatValue),
			m_UIntValue(0), m_FloatValue(v), m_Text(str), m_Size(BytesToBits(sizeof(float)))
		{ }
		Operand(std::string v) : m_Type(TextValue),
			m_Text(v), m_Size(v.size())
		{ }
		Operand(Tokens::Number::Info<Numbers::IntegerType>* info) : Operand(*info->GetValue<Tokens::Number::NumberValue>(), info->GetValue<Tokens::Number::ScriptRange>().Format())
		{ }
		Operand(Tokens::Number::Info<Numbers::FloatType>* info) : Operand(*info->GetValue<Tokens::Number::NumberValue>(), info->GetValue<Tokens::Number::ScriptRange>().Format())
		{ }

		template<typename T = int64_t>
		inline T& Value();
		template<typename T = int64_t>
		inline const T& Value() const;
		template<> inline int64_t& Value() { return m_IntValue; }
		template<> inline const int64_t& Value() const { return m_IntValue; }
		template<> inline uint64_t& Value() { return m_UIntValue; }
		template<> inline const uint64_t& Value() const { return m_UIntValue; }
		template<> inline double& Value() { return m_FloatValue; }
		template<> inline const double& Value() const { return m_FloatValue; }
		template<> inline ScriptLabel& Value() { return *m_LabelValue; }
		template<> inline const ScriptLabel& Value() const { return *m_LabelValue; }
		template<> inline ScriptVariable& Value() { return *m_VariableValue; }
		template<> inline const ScriptVariable& Value() const { return *m_VariableValue; }
		inline bool IsNumberType() const { return m_Type == IntValue || m_Type == FloatValue; }
		inline bool IsIntType() const { return m_Type == IntValue; }
		inline bool IsFloatType() const { return m_Type == FloatValue; }
		inline bool IsTextType() const { return m_Type == TextValue; }
		inline bool IsLabelType() const { return m_Type == LabelValue; }
		inline bool IsVariableType() const { return m_Type == VariableValue; }
		inline Type GetType() const { return m_Type; }
		inline std::string Text() const { return m_Text; }
		inline bool HasSize() const { return m_Size != -1; }
		inline size_t Size() const { return m_Size; }
		inline void SetSize(size_t v) { m_Size = v; }

		Attributes GetNumberAttributes() const;
		Attributes GetTextAttributes() const;
		Attributes GetLabelAttributes() const;
		Attributes GetVariableAttributes() const;

		Operand& Negate() {
			if (m_Type == Type::IntValue)
				m_IntValue = -m_IntValue;
			else if (m_Type == Type::FloatValue)
				m_FloatValue = -m_FloatValue;
			return *this;
		}

	private:
		Type m_Type = NullValue;
		union {
			uint64_t m_UIntValue = 0;
			int64_t m_IntValue;
			double m_FloatValue;
		};
		ScriptLabel* m_LabelValue = nullptr;
		ScriptVariable* m_VariableValue = nullptr;
		std::string m_Text;

		size_t m_Size = -1;
	};
}