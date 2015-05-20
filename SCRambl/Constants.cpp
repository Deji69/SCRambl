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
#include "Builder.h"

namespace SCRambl
{
	void Constants::Init(Build& build) {
		m_Config = build.AddConfig("Constants");
		auto& conf_constant = m_Config->AddClass("Constant", [this](const XMLNode xml, std::shared_ptr<void> & obj){
			if (auto attr_name = xml.GetAttribute("Name")) {
				auto name = attr_name.GetValue().AsString();
				if (auto attr_val = xml.GetAttribute("Value")) {
					auto constant = this->AddConstant<long>(name, attr_val.GetValue().AsNumber<long>());
					obj = constant;
					return;
				}
			}
			obj = nullptr;
		});
		auto& conf_enum = m_Config->AddClass("Enum", [this](const XMLNode xml, std::shared_ptr<void> & obj){
			if (auto attr_name = xml.GetAttribute("Name")) {
				auto name = attr_name.GetValue().AsString();
				obj = this->AddEnum(name);
			}
		});
		conf_enum.AddClass("Constant", [this](const XMLNode xml, std::shared_ptr<void> & obj){
			;
			if (auto attr_name = xml.GetAttribute("Name")) {
				auto name = attr_name.GetValue().AsString();
				SCR::Enumerator::Shared enumerator;
				if (auto attr_val = xml.GetAttribute("Value")) {
					enumerator = this->AddEnumerator(std::static_pointer_cast<SCR::Enum>(obj), name, attr_val.GetValue().AsNumber<long>());
				}
				else {
					enumerator = this->AddEnumerator(std::static_pointer_cast<SCR::Enum>(obj), name);
				}
			}
		});
	}
	Constants::Constants() { }
}