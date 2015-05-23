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

		void Types::Init(Build& build) {

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
				static const auto size_fun = [this](const XMLNode vec, std::shared_ptr<void> & obj){
					//auto value = std::static_pointer_cast<ValueExt>(obj);

					/*size_t size = 0;
					if (!vec.empty()) {
					Numbers::IntegerType size;
					if (Numbers::StringToInt(vec.value(), size) == Numbers::ConvertResult::success)
					value->AddSize(size);
					}*/
				};

				auto& type = m_Config->AddClass("Type", [this](const XMLNode vec, std::shared_ptr<void> & obj){
					// store the command to the object pointer so we can access it again
					auto type = AddType(vec.GetAttribute("NAME").GetValue().AsString(), TypeSet::Basic);
					obj = type;
					//std::cout << "Type: name " << type->GetName() << ", id " << type->GetID() << "\n";
				});

				auto& value = type.AddClass("Number", [this](const XMLNode vec, std::shared_ptr<void> & obj){
					auto type = std::static_pointer_cast<Type>(obj);
					std::string str = vec.GetAttribute("Type").GetValue().AsString();

					NumberValueType numtype = NumberValueType::Integer;
					if (str == "float") numtype = NumberValueType::Float;

					auto value = type->AddValue<NumberValue>(type, numtype, vec.GetAttribute("Size").GetValue().AsNumber<int>(0));
					AddValue(ValueSet::Number, value);
					obj = value;
				});
				//value.AddClass("Size", size_fun);

				auto& text = type.AddClass("Text", [this](const XMLNode vec, std::shared_ptr<void> & obj){
					auto type = std::static_pointer_cast<Type>(obj);
					auto text = type->AddValue<Value>(type, ValueSet::Text);
					//AddValue(ValueSet::Text, text);
					auto size_attr = vec.GetAttribute("Size");
					if (size_attr) {
						auto str = size_attr.GetValue().AsString();
					}
					obj = text;
				});
				auto& command = type.AddClass("Command", [this](const XMLNode vec, std::shared_ptr<void>& obj) {
					auto type = std::static_pointer_cast<Type>(obj);
					auto cmd = type->AddValue<Value>(type, ValueSet::Command);
					auto size_attr = vec.GetAttribute("Size");
					if (size_attr) {
						auto value = type->AddValue<CommandValue>(type, size_attr.GetValue().AsNumber<uint32_t>());
					}
				});
				//text.AddClass("Size", size_fun);

				/*auto& variable = type.AddClass("Variable", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				auto type = std::static_pointer_cast<SCRambl::SCR::Type>(obj);
				auto type_attr = vec.attribute("Type");
				auto value_attr = vec.attribute("Value");
				auto var_type = GetType(type_attr.as_string());
				auto val_type = GetType(value_attr.as_string());

				auto var = type->AddValue<SCR::Type::Variable>(*var_type, *val_type);
				obj = var;
				});
				auto& array = type.AddClass("Array", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				auto type = std::static_pointer_cast<SCRambl::SCR::Type>(obj);
				auto type_attr = vec.attribute("Type");
				auto value_attr = vec.attribute("Value");
				auto var_type = GetType(type_attr.as_string());
				auto val_type = GetType(value_attr.as_string());

				if (var_type && val_type)
				{
				auto array = type->AddValue<SCR::Type::Variable>(*var_type, *val_type);
				obj = array;
				}
				else obj = nullptr;
				});*/

				//AddSizeAttribute(value);
				//AddSizeAttribute(string);

				/*value.AddClass("Width", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				auto value = std::static_pointer_cast<SCRambl::SCR::Type::Number>(obj);
				bool is_variable = false;
				if (auto attr = vec.attribute("Variable"))
				is_variable = attr.as_bool(false);
				Numbers::IntegerType size;
				auto result = Numbers::StringToInt(vec.first_child().value(), size);
				if (result == Numbers::ConvertResult::success) {
				value->AddSize(size, is_variable);
				}
				});

				value.AddClass("Float", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				auto value = std::static_pointer_cast<SCRambl::SCR::Type::Number>(obj);
				value->SetFloat(true);
				});*/
			}
			m_Config = build.AddConfig("ExtendedTypes");
			{
				auto& extype = m_Config->AddClass("Type", [this](const XMLNode vec, std::shared_ptr<void> & obj){
					unsigned long id = 0;
					if (auto attr = vec.GetAttribute("ID"))
						id = attr.GetValue().AsNumber<unsigned int>();

					if (auto attr = vec.GetAttribute("Hash")) {
						std::hash<std::string> hasher;
						id = hasher(vec.GetAttribute(attr.GetValue().AsString()).GetValue().AsString());
					}

					auto type = AddType(vec.GetAttribute("Name").GetValue().AsString(), TypeSet::Extended);

					// store the type to the object pointer so we can access it again
					obj = type;
					//std::cout << "Type: name " << type->GetName() << ", id " << type->GetID() << "\n";
				});
			}
			m_Config = build.AddConfig("Translations");
			{
				auto& trans = m_Config->AddClass("Translate", [this](const XMLNode vec, std::shared_ptr<void> & obj){
					std::string type_name = vec.GetAttribute("Type").GetValue().AsString();
					if (auto type = GetType(type_name)) {
						ValueSet valtype = GetValueTypeByName(vec.GetAttribute("Value").GetValue().AsString());

						size_t size = vec.GetAttribute("Size").GetValue().AsNumber<size_t>(-1);
						auto translation = AddTranslation(type, valtype, size);

						type->AllValues<Value>([valtype, size, translation](Value::Shared value){
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
				auto& data = trans.AddClass("Data", [this](const XMLNode vec, std::shared_ptr<void> & obj){
					if (obj) {
						auto translation = std::static_pointer_cast<Translation<>>(obj);
						auto& data = translation->AddData();
						for (auto& it : vec)
						{
							std::string name = it.Name();
							DataType data_type;
							if (DataType::GetByName(name, data_type)) {
								auto src_attr = it.GetAttribute("Source");
								auto attr_attr = it.GetAttribute("Attribute");
								Translation<>::Data::Field::Shared field;

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