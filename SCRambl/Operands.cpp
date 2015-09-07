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
	if (GetType() == Type::LabelValue) {
		attrs.SetAttribute(Types::DataAttributeID::IsGlobal, (*m_LabelValue)->IsGlobal());
		attrs.SetAttribute(Types::DataAttributeID::Offset, (*m_LabelValue)->Offset());
		attrs.SetAttribute(Types::DataAttributeID::Index, (*m_LabelValue)->Index());
		attrs.SetAttribute(Types::DataAttributeID::Name, (*m_LabelValue)->Name());
	}
	return attrs;
}
Operand::Attributes Operand::GetVariableAttributes() const {
	Attributes attrs;
	if (GetType() == Type::VariableValue) {
		attrs.SetAttribute(Types::DataAttributeID::ID, (*m_VariableValue)->ID());
		attrs.SetAttribute(Types::DataAttributeID::Index, (*m_VariableValue)->Index());
		attrs.SetAttribute(Types::DataAttributeID::Offset, (*m_VariableValue)->Offset());
		attrs.SetAttribute(Types::DataAttributeID::Name, (*m_VariableValue)->Name());
		attrs.SetAttribute(Types::DataAttributeID::Size, (*m_VariableValue)->Value()->GetSize());
		attrs.SetAttribute(Types::DataAttributeID::IsArray, (*m_VariableValue)->IsArray());
		attrs.SetAttribute(Types::DataAttributeID::IsGlobal, (*m_VariableValue)->IsGlobal());
	}
	return attrs;
}