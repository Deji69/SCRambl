#include "stdafx.h"
#include "Types.h"
#include "Engine.h"
#include "Numbers.h"

namespace SCRambl
{
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

	Types::Types(Engine & eng) : m_Engine(eng)
	{
		m_Config = m_Engine.AddConfig("VariableTypes");
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
				}*/
			});
			vartype.AddClass("Array", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				auto vartype = std::static_pointer_cast<SCR::VarType<>>(obj);
				vartype->SetIsArray(true);
			});
		}
		m_Config = m_Engine.AddConfig("Types");
		{
			auto& type = m_Config->AddClass("Type", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				// store the command to the object pointer so we can access it again
				unsigned long id = 0;
				if (auto attr = vec.attribute("ID"))
					id = attr.as_uint();

				if (auto attr = vec.attribute("Hash")) {
					std::hash<std::string> hasher;
					id = hasher(vec.attribute(attr.as_string()).as_string());
				}

				auto type = AddType(vec.attribute("Name").as_string(), id);
				obj = type;

				//std::cout << "Type: name " << type->GetName() << ", id " << type->GetID() << "\n";
			});

			auto& value = type.AddClass("Value", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				auto type = std::static_pointer_cast<SCRambl::SCR::Type>(obj);
				auto value = type->AddValue<SCR::Type::Value>();
				obj = value;
			});
			auto& string = type.AddClass("String", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				auto type = std::static_pointer_cast<SCRambl::SCR::Type>(obj);
				auto string = type->AddValue<SCR::Type::String>();
				obj = string;
			});
			auto& variable = type.AddClass("Variable", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
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
			});

			AddSizeAttribute(value);
			AddSizeAttribute(string);

			value.AddClass("Width", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				auto value = std::static_pointer_cast<SCRambl::SCR::Type::Value>(obj);
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
				auto value = std::static_pointer_cast<SCRambl::SCR::Type::Value>(obj);
				value->SetFloat(true);
			});
		}
		m_Config = m_Engine.AddConfig("ExtendedTypes");
		{
			auto& extype = m_Config->AddClass("Type", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				// store the command to the object pointer so we can access it again
				unsigned long id = 0;
				if (auto attr = vec.attribute("ID"))
					id = attr.as_uint();

				if (auto attr = vec.attribute("Hash")) {
					std::hash<std::string> hasher;
					id = hasher(vec.attribute(attr.as_string()).as_string());
				}

				auto type = AddType(vec.attribute("Name").as_string(), id);
				obj = type;

				//std::cout << "Type: name " << type->GetName() << ", id " << type->GetID() << "\n";
			});
		}
	}
}