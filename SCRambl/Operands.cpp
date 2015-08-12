#include "stdafx.h"
#include "Operands.h"

using namespace SCRambl;

Operand::Attributes Operand::GetNumberAttributes() const {
	Attributes attrs;
	if (GetType() == Type::FloatValue || GetType() == Type::IntValue) {
		auto size = CountByteOccupation(Value<unsigned long long>());
		attrs.SetAttribute(Types::DataAttributeID::Size, size);
		if (GetType() == Type::FloatValue)
			attrs.SetAttribute(Types::DataAttributeID::Value, Value<float>());
		else
			attrs.SetAttribute(Types::DataAttributeID::Value, Value<int64_t>());
	}
	return attrs;
}
Operand::Attributes Operand::GetTextAttributes() const {
	Attributes attrs;
	if (GetType() == Type::TextValue) {
		attrs.SetAttribute(Types::DataAttributeID::Size, m_Text.size());
		attrs.SetAttribute(Types::DataAttributeID::Value, m_Text);
	}
	return attrs;
}
Operand::Attributes Operand::GetLabelAttributes() const {
	Attributes attrs;
	attrs.SetAttribute(Types::DataAttributeID::Offset, 0);
	attrs.SetAttribute(Types::DataAttributeID::Name, m_LabelValue->Get().Name());
	BREAK();
	return attrs;
}
Operand::Attributes Operand::GetVariableAttributes() const {
	Attributes attrs;
	if (GetType() == Type::VariableValue) {
		attrs.SetAttribute(Types::DataAttributeID::Index, m_VariableValue->Index());
		attrs.SetAttribute(Types::DataAttributeID::Name, m_VariableValue->Get().Name());
		attrs.SetAttribute(Types::DataAttributeID::Size, m_VariableValue->Get().Value()->GetSize());
		attrs.SetAttribute(Types::DataAttributeID::IsArray, m_VariableValue->Get().IsArray());
		attrs.SetAttribute(Types::DataAttributeID::IsGlobal, m_VariableValue->Get().IsGlobal());
	}
	return attrs;
}