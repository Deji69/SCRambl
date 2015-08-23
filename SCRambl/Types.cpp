#include "stdafx.h"
#include "Types.h"
#include "Engine.h"
#include "Numbers.h"
#include "Text.h"

namespace SCRambl
{
	const Types::DataSourceSet Attributes<Types::DataSourceID, Types::DataSourceSet>::s_AttributeSet;
	const Types::DataAttributeSet Attributes<Types::DataAttributeID, Types::DataAttributeSet>::s_AttributeSet;
	
	namespace Types
	{
		const DataAttributesMap Xlation::s_NullMap;
		const Translation::Ref Translation::BadRef;
		
		std::string DataSource::GetNameByID(DataSourceID id) {
			switch (id) {
			case DataSourceID::Env:
				return "Env";
			case DataSourceID::Value:
				return "Value";
			case DataSourceID::Number:
				return "Number";
			case DataSourceID::Text:
				return "Text";
			case DataSourceID::Command:
				return "Command";
			case DataSourceID::Variable:
				return "Variable";
			case DataSourceID::Label:
				return "Label";
			case DataSourceID::Condition:
				return "Condition";
			}
			return "";
		}
		DataSourceID DataSource::GetIDByName(std::string id) {
			static const std::map<std::string, DataSourceID> map = {
				{ "Env", DataSourceID::Env },
				{ "Value", DataSourceID::Value },
				{ "Number", DataSourceID::Number },
				{ "Text", DataSourceID::Text },
				{ "Command", DataSourceID::Command },
				{ "Variable", DataSourceID::Variable },
				{ "Condition", DataSourceID::Condition },
			};
			auto it = map.find(id);
			return it != map.end() ? it->second : DataSourceID::None;
		}
		std::string DataAttribute::GetNameByID(DataAttributeID id) {
			switch (id) {
			case DataAttributeID::Value:
				return "Value";
			case DataAttributeID::Size:
				return "Size";
			case DataAttributeID::Offset:
				return "Offset";
			case DataAttributeID::ID:
				return "ID";
			case DataAttributeID::Index:
				return "Index";
			case DataAttributeID::Name:
				return "Name";
			case DataAttributeID::NumArgs:
				return "NumArgs";
			case DataAttributeID::IsNOT:
				return "IsNOT";
			}
			return "";
		}
		DataAttributeID DataAttribute::GetIDByName(std::string id) {
			static const std::map<std::string, DataAttributeID> map = {
				{ "Value", DataAttributeID::Value },
				{ "Size", DataAttributeID::Size },
				{ "Offset", DataAttributeID::Offset },
				{ "ID", DataAttributeID::ID },
				{ "Index", DataAttributeID::Index },
				{ "Name", DataAttributeID::Name },
				{ "NumArgs", DataAttributeID::NumArgs },
				{ "IsNOT", DataAttributeID::IsNOT },
			};
			auto it = map.find(id);
			return it != map.end() ? it->second : DataAttributeID::None;
		}
		ValueSet GetValueTypeByName(std::string name) {
			static const std::unordered_map<std::string, ValueSet> table = {
				{ "Null", ValueSet::Null },
				{ "Number", ValueSet::Number },
				{ "Variable", ValueSet::Variable },
				{ "Array", ValueSet::Array },
				{ "Text", ValueSet::Text },
				{ "Label", ValueSet::Label },
				{ "Command", ValueSet::Command },
			};
			auto it = table.find(name);
			return it == table.end() ? ValueSet::INVALID : it->second;
		}

		/* VariableValue */
		inline ArrayValue* VariableValue::ToArray() {
			ASSERT(IsArray());
			return static_cast<ArrayValue*>(this);
		}
		inline const ArrayValue* VariableValue::ToArray() const {
			ASSERT(IsArray());
			return static_cast<const ArrayValue*>(this);
		}
		VariableValue::VariableValue(VecRef<Type> type, size_t size, VecRef<Variable> var, VecRef<Type> val, bool) : Value(type, ValueSet::Array, size),
			m_VarType(var), m_ValType(val)
		{ }
		VariableValue::VariableValue(VecRef<Type> type, size_t size, VecRef<Variable> var, VecRef<Type> val) : Value(type, ValueSet::Variable, size),
			m_VarType(var), m_ValType(val)
		{ }

		/* ArrayValue */
		ArrayValue::ArrayValue(VecRef<Type> type, size_t size, VecRef<Variable> var, VecRef<Type> val) : VariableValue(type, size, var, val, true)
		{ }

		/* Type */
		size_t Type::GetID() const { return m_ID; }
		std::string Type::GetName() const { return m_Name; }
		TypeSet Type::GetType() const { return m_Type; }
		MatchLevel Type::GetMatchLevel(const Type* type) const {
			if (type == this) return MatchLevel::Strict;
			if (auto valtype = type->GetValueType()) {
				if (type == valtype) return MatchLevel::Basic;
			}
			return MatchLevel::None;
		}
		const Type* Type::GetValueType() const {
			if (IsVariableType()) {
				if (auto vt = ToVariable()->GetVarValue()) {
					return vt->GetType();
				}
			}
			else if (IsExtendedType()) {
				return ToExtended()->GetBasicType();
			}
			else if (IsBasicType()) {
				return this;
			}
			return nullptr;
		}
		VariableValue* Type::GetVarValue() const {
			VariableValue* value = nullptr;
			AllValues<Value>([&value](Value* val) {
				if (val->GetValueType() == ValueSet::Variable) {
					value = &val->Extend<VariableValue>();
					return true;
				}
				return false;
			});
			return value ? value : nullptr;
		}
		ArrayValue* Type::GetArrayValue() const {
			ArrayValue* value = nullptr;
			AllValues<Value>([&value](Value* val){
				if (val->GetValueType() == ValueSet::Array) {
					value = &val->Extend<ArrayValue>();
					return true;
				}
				return false;
			});
			return value ? value : nullptr;
		}
		size_t Type::GetVarMinIndex() const { return IsVariableType() ? ToVariable()->MinIndex().AsNumber<size_t>() : 0; }
		size_t Type::GetVarMaxIndex() const { return IsVariableType() ? ToVariable()->MaxIndex().AsNumber<size_t>() : 0; }
		bool Type::HasValueType(ValueSet type) const {
			for (auto& val : m_Values) {
				if (val->GetValueType() == type)
					return true;
			}
			return false;
		}
		bool Type::IsGlobalVar() const { return IsVariableType() && ToVariable()->IsGlobal(); }
		bool Type::IsScopedVar() const { return IsVariableType() && !ToVariable()->IsGlobal(); }
		bool Type::IsBasicType() const { return GetType() == TypeSet::Basic; }
		bool Type::IsExtendedType() const { return GetType() == TypeSet::Extended; }
		bool Type::IsVariableType() const { return GetType() == TypeSet::Variable; }
		Basic* Type::ToBasic() { return IsBasicType() ? static_cast<Basic*>(this) : nullptr; }
		Extended* Type::ToExtended() { return IsExtendedType() ? static_cast<Extended*>(this) : nullptr; }
		Variable* Type::ToVariable() { return IsVariableType() ? static_cast<Variable*>(this) : nullptr; }
		const Basic* Type::ToBasic() const { return IsBasicType() ? static_cast<const Basic*>(this) : nullptr; }
		const Extended* Type::ToExtended() const { return IsExtendedType() ? static_cast<const Extended*>(this) : nullptr; }
		const Variable* Type::ToVariable() const { return IsVariableType() ? static_cast<const Variable*>(this) : nullptr; }
		void Type::CopyValues(const Type& other) {
			for (auto val : other.m_Values) {
				m_Values.emplace_back(val);
			}
		}
		void Type::MoveValues(Type&& other) {
			for (auto val : other.m_Values) {
				m_Values.emplace_back(val);
			}
			other.m_Values.clear();
		}
		Type::Type(Type&& type) : m_ID(type.m_ID), m_Name(type.m_Name), m_Type(type.m_Type) {
			MoveValues(std::forward<Type>(type));
		}

		/* DataType */
		DataType::Type DataType::GetByName(std::string name) {
			static const std::map<std::string, Type> map = {
				{ "int", Int }, { "float", Float }, { "fixed", Fixed },
				{ "char", Char }, { "string", String }
			};
			auto it = map.find(name);
			return it != map.end() ? it->second : INVALID;
		}
		bool DataType::GetByName(std::string str, Type& out, size_t& size_out) {
			static const std::map<std::string, Type> map = {
				{ "Int", Int }, { "Float", Float }, { "Fixed", Fixed },
				{ "Char", Char }, { "String", String }
			};
			std::string name = strcpy_while(str.begin(), str.end(), [](char c){ return !isdigit(c); });

			auto it = map.find(name);
			if (it != map.end()) {
				out = it->second;
				if (name.size() != str.size()) {
					size_out = std::stol(str.substr(name.size()));
				}
				return true;
			}
			return false;
		}

		/* Types */
		Translation::Ref Types::AddTranslation(TypeRef<Type> type, ValueSet valuetype, size_t size) {
			auto idx = m_Translations.size();
			m_Translations.emplace_back(type.Ref(), valuetype, size);
			return m_Translations.size() != idx ? Translation::Ref(m_Translations, idx) : Translation::BadRef;
		}
		void Types::AddValueAttributes(XMLConfig* type) {
			auto value = type->AddClass("Number", [this](const XMLNode vec, void*& obj){
				auto type = static_cast<Type*>(obj);
				NumberValueType numtype = vec["Type"]->AsString() == "float" ? numtype = NumberValueType::Float : NumberValueType::Integer;

				auto value = type->AddValue<NumberValue>(m_Types.Get(type->GetID()).Ref(), numtype, vec["Size"]->AsNumber<uint32_t>(0));
				AddValue(ValueSet::Number, value);
				obj = value;
			});
			auto text = type->AddClass("Text", [this](const XMLNode vec, void*& obj){
				auto type = static_cast<Type*>(obj);
				auto value = type->AddValue<TextValue>(m_Types.Get(type->GetID()).Ref(), vec["Size"]->AsNumber<uint32_t>(), *vec["Mode"]);
				AddValue(ValueSet::Text, value);
				obj = value;
			});
			auto command = type->AddClass("Command", [this](const XMLNode vec, void*& obj) {
				auto type = static_cast<Type*>(obj);
				auto datatype = DataType::GetByName(vec["Type"]->AsString());
				if (datatype == DataType::INVALID)
					BREAK();
				auto value = type->AddValue<CommandValue>(m_Types.Get(type->GetID()).Ref(), vec["Size"]->AsNumber<uint32_t>(), vec["Value"]->AsString(), datatype);
				AddValue(ValueSet::Command, value);
				obj = value;
			});
			auto label = type->AddClass("Label", [this](const XMLNode vec, void*& obj) {
				auto type = static_cast<Type*>(obj);
				auto value = type->AddValue<LabelValue>(m_Types.Get(type->GetID()).Ref(), vec["Size"]->AsNumber<uint32_t>());
				AddValue(ValueSet::Label, value);
				obj = value;
			});
			auto variable = type->AddClass("Variable", [this](const XMLNode vec, void*& obj) {
				auto type = static_cast<Type*>(obj);
				auto type_attr = vec["Type"]->AsString();
				auto value_attr = vec["Value"]->AsString();
				obj = nullptr;

				if (type_attr.empty()) {
					// error: type attribute not specified
					BREAK();
				}
				else if (value_attr.empty()) {
					// error: value attribute not specified
					BREAK();
				}
				else {
					auto vartype = GetType<Variable>(type_attr);
					auto valtype = GetType(value_attr);
					ASSERT(vartype && valtype);
					ASSERT(vartype.IsVariable());

					auto value = type->AddValue<VariableValue>(m_Types.Get(type->GetID()).Ref(), vec["Size"]->AsNumber<uint32_t>(), vartype.Ref(), valtype.Ref());
					AddValue(ValueSet::Variable, value);
					obj = value;
				}
			});
			auto array_var = type->AddClass("Array", [this](const XMLNode vec, void*& obj) {
				auto type = static_cast<Type*>(obj);
				auto type_attr = vec["Type"]->AsString();
				auto value_attr = vec["Value"]->AsString();
				obj = nullptr;

				if (type_attr.empty()) {
					// error: type attribute not specified
					BREAK();
				}
				else if (value_attr.empty()) {
					// error: value attribute not specified
					BREAK();
				}
				else {
					auto vartype = GetType<Variable>(type_attr);
					auto valtype = GetType(value_attr);

					ASSERT(vartype && valtype);
					ASSERT(vartype.IsVariable() && vartype.AsVariable().IsArray());

					auto value = type->AddValue<ArrayValue>(m_Types.Get(type->GetID()).Ref(), vec["Size"]->AsNumber<uint32_t>(), vartype.Ref(), valtype.Ref());
					AddValue(ValueSet::Array, value);
					obj = value;
				}
			});
		}
		void Types::Init(Build& build) {

			m_Config = build.AddConfig("VariableTypes");
			{
				auto vartype = m_Config->AddClass("Type", [this](const XMLNode vec, void*& obj) {
					auto scope = *vec["Scope"];
					auto isarray = *vec["IsArray"];
					auto name = vec["Name"]->AsString();
					auto size = vec["Size"]->AsNumber<size_t>(32);
					auto type = AddVariableType(name, scope, isarray.AsBool(false), size);

					auto rg = m_ValsToUpdate.equal_range(name);
					if (rg.first != rg.second) {
						for (auto it = rg.first; it != rg.second; ++it) {
							if (it->second.isArray != isarray.AsBool(false))
								BREAK();
							if (it->second.isValue)
								BREAK();

							it->second.varType.m_VarType = type;
						}
					}

					obj = type;
				});
				vartype->AddClass("MinIndex", [this](const XMLNode vec, void*& obj) {
					auto type = static_cast<Variable*>(obj);
					type->SetMinIndex(vec->AsNumber<size_t>());
				});
				vartype->AddClass("MinIndex", [this](const XMLNode vec, void*& obj) {
					auto type = static_cast<Variable*>(obj);
					type->SetMaxIndex(vec->AsNumber<size_t>());
				});
			}

			m_Config = build.AddConfig("BasicTypes");
			{
				auto type = m_Config->AddClass("Type", [this](const XMLNode vec, void*& obj){
					// store the command to the object pointer so we can access it again
					auto name = vec["Name"]->AsString();
					auto type = AddType(name);

					auto rg = m_ValsToUpdate.equal_range(name);
					if (rg.first != rg.second) {
						for (auto it = rg.first; it != rg.second; ++it) {
							if (!it->second.isValue)
								BREAK();

							it->second.varType.m_ValueType = type;
						}
					}

					obj = type;
					//std::cout << "Type: name " << type->GetName() << ", id " << type->GetID() << "\n";
				});
				AddValueAttributes(type);
			}
			m_Config = build.AddConfig("ExtendedTypes");
			{
				auto extype = m_Config->AddClass("Type", [this](const XMLNode vec, void*& obj){
					unsigned long id = 0;
					if (auto attr = vec["ID"])
						id = attr.GetValue().AsNumber<unsigned int>();

					if (auto attr = vec["Hash"]) {
						std::hash<std::string> hasher;
						id = hasher(vec[attr->AsString()]->AsString());
					}

					auto name = vec["Name"]->AsString();
					auto type = AddExtendedType(name);

					auto rg = m_ValsToUpdate.equal_range(name);
					if (rg.first != rg.second) {
						for (auto it = rg.first; it != rg.second; ++it) {
							if (!it->second.isValue)
								BREAK();

							it->second.varType.m_ValueType = type;
						}
					}

					// store the type to the object pointer so we can access it again
					obj = type;
					//std::cout << "Type: name " << type->GetName() << ", id " << type->GetID() << "\n";
				});
				AddValueAttributes(extype);
			}
			m_Config = build.AddConfig("Translations");
			{
				auto trans = m_Config->AddClass("Translate", [this](const XMLNode vec, void*& obj){
					auto type_name = vec["Type"]->AsString();
					if (auto type = GetType(type_name)) {
						auto val = *vec["Value"];
						auto valtype = GetValueTypeByName(val.AsString());

						auto size = vec["Size"]->AsNumber<size_t>(-1);
						auto translation = AddTranslation(type, valtype, size);

						type.Get().AllValues<Value>([valtype, size, translation](Value* value){
							if (size == -1 || value->GetSize() == size) {
								if (valtype == ValueSet::INVALID || valtype == value->GetValueType())
									value->SetTranslation(translation);
							}
							return false;
						});

						obj = translation.Ptr();
						return;
					}
					else {
						BREAK();
					}

					obj = nullptr;
				});
				auto data = trans->AddClass("Data", [this](const XMLNode vec, void*& obj){
					if (obj) {
						auto translation = static_cast<Translation*>(obj);
						auto& data = translation->AddData();
						for (auto& it : vec)
						{
							std::string name = it.Name();
							DataType data_type;
							size_t size;
							Translation::Data::Field* field;
							if (name == "Args") {
								field = data.AddField(DataType::Args, DataSourceID::None, DataAttributeID::None);
							}
							else if (DataType::GetByName(name, data_type, size)) {
								auto src_attr = it["Source"];
								auto attr_attr = it["Attribute"];

								if (src_attr && attr_attr) {
									auto src_id = GetDataSource(src_attr->AsString());
									auto attr_id = GetDataAttribute(src_id, attr_attr->AsString());
									if (src_id == DataSourceID::None || attr_id == DataAttributeID::None) {
										BREAK();
										field = data.AddField(data_type, DataSourceID::None, DataAttributeID::None, size);
									}
									else {
										field = data.AddField(data_type, src_id, attr_id, size);
									}
								}
								else field = data.AddField(data_type, DataSourceID::None, DataAttributeID::None, size);

								if (!it->AsString().empty())
								{
									if (data_type.IsInteger() || data_type.IsFloat()) {
										SCRambl::Numbers::IntegerType int_num;
										SCRambl::Numbers::FloatType flt_num;
										if (data_type.IsInteger()) {
											auto txt = *it;
											auto convert_result = Numbers::StringToInt<long long>(it->AsString().c_str(), int_num, true);
											if (convert_result == Numbers::ConvertResult::success) {
												field->SetValue(int_num.GetValue<long long>());
											}
											else {
												BREAK();
												field->SetValue(0);
											}
										}
										else if (data_type.IsFloat()) {
											auto convert_result = Numbers::StringToFloat<float>(it->AsString().c_str(), flt_num, true);
											if (convert_result == Numbers::ConvertResult::success) {
												field->SetValue(flt_num.GetValue<float>());
											}
											else {
												BREAK();
												field->SetValue(0.0);
											}
										}
									}
									else if (data_type.IsString()) {
										field->SetValue(*it);
									}
									else {
										BREAK();
										field->SetValue(0);
									}
								}
							}
						}
					}
				});
			}
		}
		Types::Types()
		{ }
	}
}