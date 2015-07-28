#include "stdafx.h"
#include "Operands.h"

using namespace SCRambl;

Operand::Attributes Operand::GetNumberAttributes() const {
	Attributes attrs;
	attrs.SetAttribute(Types::DataAttributeID::Size, CountByteOccupation(Value<int64_t>()));
	attrs.SetAttribute(Types::DataAttributeID::Value, Value<int64_t>());
	return attrs;
}
Operand::Attributes Operand::GetTextAttributes() const {
	Attributes attrs;
	attrs.SetAttribute(Types::DataAttributeID::Size, m_Text.size());
	attrs.SetAttribute(Types::DataAttributeID::Value, m_Text);
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
	attrs.SetAttribute(Types::DataAttributeID::Index, 0);
	attrs.SetAttribute(Types::DataAttributeID::Name, m_VariableValue->Get().Name());
	attrs.SetAttribute(Types::DataAttributeID::Size, m_VariableValue->Get().Value()->GetSize());
	attrs.SetAttribute(Types::DataAttributeID::IsArray, m_VariableValue->Get().IsArray());
	attrs.SetAttribute(Types::DataAttributeID::IsGlobal, m_VariableValue->Get().IsGlobal());
	BREAK();
	return attrs;
}