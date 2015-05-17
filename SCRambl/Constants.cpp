/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#include "stdafx.h"
#include "Constants.h"
#include "Engine.h"
#include "SCR.h"

namespace SCRambl
{
	Constants::Constants(Engine & eng) : m_Engine(eng) {
		m_Config = eng.AddConfiguration("Constants");
		auto& conf_constant = m_Config->AddClass("Constant", [this](const pugi::xml_node xml, std::shared_ptr<void> & obj){
			auto attr_name = xml.attribute("Name");
			if (!attr_name.empty()) {
				auto name = attr_name.as_string();
				auto attr_val = xml.attribute("Value");
				if (!attr_val.empty()) {
					auto constant = this->AddConstant<long>(name, attr_val.as_int());
					obj = constant;
					return;
				}
			}
			obj = nullptr;
		});
		auto& conf_enum = m_Config->AddClass("Enum", [this](const pugi::xml_node xml, std::shared_ptr<void> & obj){
			auto attr_name = xml.attribute("Name");
			if (!attr_name.empty()) {
				auto name = attr_name.as_string();
				obj = this->AddEnum(name);
			}
		});
		conf_enum.AddClass("Constant", [this](const pugi::xml_node xml, std::shared_ptr<void> & obj){
			auto attr_name = xml.attribute("Name");
			if (!attr_name.empty()) {
				auto name = attr_name.as_string();
				auto attr_val = xml.attribute("Value");
				SCR::Enumerator::Shared enumerator;
				if (!attr_val.empty()) {
					enumerator = this->AddEnumerator(std::static_pointer_cast<SCR::Enum>(obj), name, attr_val.as_int());
				}
				else {
					enumerator = this->AddEnumerator(std::static_pointer_cast<SCR::Enum>(obj), name);
				}
			}
		});
	}
}