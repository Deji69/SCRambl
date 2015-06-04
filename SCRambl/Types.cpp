#include "stdafx.h"
#include "Types.h"
#include "Engine.h"
#include "Numbers.h"
#include "Text.h"

namespace SCRambl
{
	namespace Types
	{
		ValueSet GetValueTypeByName(std::string name) {
			static const std::map<std::string, ValueSet> table = {
				{ "Null", ValueSet::Null },
				{ "Number", ValueSet::Number },
				{ "Variable", ValueSet::Variable },
				{ "Text", ValueSet::Text },
				{ "Label", ValueSet::Label }
			};
			auto it = table.find(name);
			return it == table.end() ? ValueSet::INVALID : it->second;
		}

#if 0
		void AddSizeAttribute(SCRambl::Configuration::Config & obj) {
			obj.AddClass("Size", [](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				auto value = std::static_pointer_cast<SCR::Type::AnonymousValue>(obj);
				bool is_variable = false;
				if (auto attr = vec.attribute("Variable"))
					is_variable = attr.as_bool(false);
				Numbers::IntegerType size;
				auto result = Numbers::StringToInt(vec.first_child().value(), size);
				if (result == Numbers::ConvertResult::success) {
					value->AddSize(size * 8, is_variable);
				}
			});
		}
#endif

		/* Type */
		bool Type::HasValueType(ValueSet type) const {
			for (auto& val : m_Values) {
				if (val->GetValueType() == type)
					return true;
			}
			return false;
		}
		Basic* Type::ToBasic() { return IsBasicType() ? static_cast<Basic*>(this) : nullptr; }
		Extended* Type::ToExtended() { return IsBasicType() ? static_cast<Extended*>(this) : nullptr; }
		Variable* Type::ToVariable() { return IsBasicType() ? static_cast<Variable*>(this) : nullptr; }
		const Basic* Type::ToBasic() const { return IsBasicType() ? static_cast<const Basic*>(this) : nullptr; }
		const Extended* Type::ToExtended() const { return IsBasicType() ? static_cast<const Extended*>(this) : nullptr; }
		const Variable* Type::ToVariable() const { return IsBasicType() ? static_cast<const Variable*>(this) : nullptr; }
		bool Type::IsBasicType() const { return GetType() == TypeSet::Basic; }
		bool Type::IsExtendedType() const { return GetType() == TypeSet::Extended; }
		bool Type::IsVariableType() const { return GetType() == TypeSet::Variable; }

		/* Types */
		void Types::AddValueAttributes(XMLConfig& type) {
			auto& value = type.AddClass("Number", [this](const XMLNode vec, void*& obj){
				auto type = static_cast<Type*>(obj);
				NumberValueType numtype = vec["Type"]->AsString() == "float" ? numtype = NumberValueType::Float : NumberValueType::Integer;

				auto value = type->AddValue<NumberValue>(type, numtype, vec["Size"]->AsNumber<uint32_t>(0));
				AddValue(ValueSet::Number, value);
				obj = value;
			});
			auto& text = type.AddClass("Text", [this](const XMLNode vec, void*& obj){
				auto type = static_cast<Type*>(obj);
				auto value = type->AddValue<TextValue>(type, vec["Size"]->AsNumber<uint32_t>(), *vec["Mode"]);
				AddValue(ValueSet::Text, value);
				obj = value;
			});
			auto& command = type.AddClass("Command", [this](const XMLNode vec, void*& obj) {
				auto type = static_cast<Type*>(obj);
				auto value = type->AddValue<CommandValue>(type, vec["Size"]->AsNumber<uint32_t>());
				AddValue(ValueSet::Command, value);
				obj = value;
			});
			auto& label = type.AddClass("Label", [this](const XMLNode vec, void*& obj) {
				auto type = static_cast<Type*>(obj);
				auto value = type->AddValue<LabelValue>(type, vec["Size"]->AsNumber<uint32_t>());
				AddValue(ValueSet::Label, value);
				obj = value;
			});
			auto& variable = type.AddClass("Variable", [this](const XMLNode vec, void*& obj) {
				auto type = static_cast<Type*>(obj);
				auto value = type->AddValue<VariableValue>(type, vec["Size"]->AsNumber<uint32_t>());
				AddValue(ValueSet::Variable, value);
				obj = value;
			});
			auto& array_var = type.AddClass("Array", [this](const XMLNode vec, void*& obj) {
				auto type = static_cast<Type*>(obj);
				auto value = type->AddValue<ArrayValue>(type, vec["Size"]->AsNumber<uint32_t>());
				AddValue(ValueSet::Array, value);
				obj = value;
			});
		}

		void Types::Init(Build& build) {

			m_Config = build.AddConfig("VariableTypes");
			{
				auto& vartype = m_Config->AddClass("VariableTypes", [this](const XMLNode vec, void*& obj) {
					auto scope = vec.GetAttribute("Scope").GetValue();
					auto isarray = vec.GetAttribute("IsArray").GetValue();
					auto type = AddVariableType(vec.GetAttribute("Name").GetValue().AsString(), scope, isarray.AsBool(false));
					obj = type;
				});
			}

			/*m_Config = m_Engine.AddConfig("VariableTypes");
			{
			auto& vartype = m_Config->AddClass("Type", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
			// store the command to the object pointer so we can access it again
			SCR::VarScope scope;
			unsigned long id = 0;
			if (auto attr = vec.attribute("ID"))
				id = attr.as_uint();

			std::string name = vec.attribute("Name").as_string();

			if (auto attr = vec.attribute("Scope"))
			scope = SCR::VarScope(attr.as_string());

			std::hash<std::string> hasher;
			if (auto attr = vec.attribute("Hash"))
			id = hasher(vec.attribute(attr.as_string()).as_string());
			else
			id = hasher(name);

			auto type = AddVariableType(name, id, scope);
			obj = type;
			});
			vartype.AddClass("Width", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
			/*auto vartype = std::static_pointer_cast<SCRambl::SCR::VarType<>>(obj);
			bool is_variable = false;
			if (auto attr = vec.attribute("Variable"))
			is_variable = attr.as_bool(false);
			Numbers::IntegerType size;
			auto result = Numbers::StringToInt(vec.first_child().value(), size);
			if (result == Numbers::ConvertResult::success) {
			value->AddSize(size, is_variable);
			}*\/
			});
			vartype.AddClass("Array", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
			auto vartype = std::static_pointer_cast<SCR::VarType<>>(obj);
			vartype->SetIsArray(true);
			});
			}*/
			
			
			m_Config = build.AddConfig("BasicTypes");
			{
				static const auto size_fun = [this](const XMLNode vec, void*& obj){
					//auto value = std::static_pointer_cast<ValueExt>(obj);

					/*size_t size = 0;
					if (!vec.empty()) {
					Numbers::IntegerType size;
					if (Numbers::StringToInt(vec.value(), size) == Numbers::ConvertResult::success)
					value->AddSize(size);
					}*/
				};

				auto& type = m_Config->AddClass("Type", [this](const XMLNode vec, void*& obj){
					// store the command to the object pointer so we can access it again
					auto type = AddType(vec.GetAttribute("Name").GetValue().AsString());
					obj = type;
					//std::cout << "Type: name " << type->GetName() << ", id " << type->GetID() << "\n";
				});
				AddValueAttributes(type);
			}
			m_Config = build.AddConfig("ExtendedTypes");
			{
				auto& extype = m_Config->AddClass("Type", [this](const XMLNode vec, void*& obj){
					unsigned long id = 0;
					if (auto attr = vec.GetAttribute("ID"))
						id = attr.GetValue().AsNumber<unsigned int>();

					if (auto attr = vec.GetAttribute("Hash")) {
						std::hash<std::string> hasher;
						id = hasher(vec.GetAttribute(attr.GetValue().AsString()).GetValue().AsString());
					}

					auto type = AddExtendedType(vec.GetAttribute("Name").GetValue().AsString());

					// store the type to the object pointer so we can access it again
					obj = type;
					//std::cout << "Type: name " << type->GetName() << ", id " << type->GetID() << "\n";
				});
				AddValueAttributes(extype);
			}
			m_Config = build.AddConfig("Translations");
			{
				auto& trans = m_Config->AddClass("Translate", [this](const XMLNode vec, void*& obj){
					std::string type_name = vec.GetAttribute("Type").GetValue().AsString();
					if (auto type = GetType(type_name)) {
						ValueSet valtype = GetValueTypeByName(vec.GetAttribute("Value").GetValue().AsString());

						size_t size = vec.GetAttribute("Size").GetValue().AsNumber<size_t>(-1);
						auto translation = AddTranslation(type, valtype, size);

						type->AllValues<Value>([valtype, size, translation](Value* value){
							if (size == -1 || value->GetSize() == size)
							{
								if (valtype == ValueSet::INVALID || valtype == value->GetValueType())
									value->SetTranslation(translation);
							}
						});

						obj = translation;
						return;
					}
					else {
						BREAK();
					}

					obj = nullptr;
				});
				auto& data = trans.AddClass("Data", [this](const XMLNode vec, void*& obj){
					if (obj) {
						auto translation = static_cast<Translation*>(obj);
						auto& data = translation->AddData();
						for (auto& it : vec)
						{
							std::string name = it.Name();
							DataType data_type;
							if (DataType::GetByName(name, data_type)) {
								auto src_attr = it.GetAttribute("Source");
								auto attr_attr = it.GetAttribute("Attribute");
								Translation::Data::Field* field;

								if (src_attr && attr_attr) {
									auto src_id = GetDataSource(src_attr.GetValue().AsString());
									auto attr_id = GetDataAttribute(src_id, attr_attr.GetValue().AsString());
									if (src_id == DataSourceID::None || attr_id == DataAttributeID::None) {
										BREAK();
										field = data.AddField(data_type, DataSourceID::None, DataAttributeID::None);
									}
									else {
										field = data.AddField(data_type, src_id, attr_id);
									}
								}
								else field = data.AddField(data_type, DataSourceID::None, DataAttributeID::None);

								if (!it.GetValue().AsString().empty())
								{
									if (data_type.IsInteger() || data_type.IsFloat()) {
										SCRambl::Numbers::IntegerType int_num;
										SCRambl::Numbers::FloatType flt_num;
										if (data_type.IsInteger()) {
											auto txt = it.GetValue();
											auto convert_result = Numbers::StringToInt<long long>(it.GetValue().AsString().c_str(), int_num, true);
											if (convert_result == Numbers::ConvertResult::success) {
												field->SetValue(int_num);
											}
											else {
												BREAK();
												field->SetValue(0);
											}
										}
										else if (data_type.IsFloat()) {
											auto convert_result = Numbers::StringToFloat<float>(it.GetValue().AsString().c_str(), flt_num, true);
											if (convert_result == Numbers::ConvertResult::success) {
												field->SetValue(flt_num);
											}
											else {
												BREAK();
												field->SetValue(0);
											}
										}
									}
									else if (data_type.IsString()) {
										field->SetValue<std::string>(it.GetValue().AsString());
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