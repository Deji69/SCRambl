#include "stdafx.h"
#include "Types.h"
#include "Commands.h"
#include "Engine.h"

namespace SCRambl
{
	Commands::Commands()
	{ }
	Commands::Commands(Engine & eng)
	{
		auto& types = eng.GetTypes();
		auto& usecc = m_UseCaseConversion;
		auto& ccdest = m_DestCasing;
		auto& ccsrc = m_SourceCasing;

		m_Config = eng.AddConfig("Commands");

		m_Config->AddClass("Case", [this, &usecc, &ccdest, &ccsrc](const pugi::xml_node xml, std::shared_ptr<void> & obj){
			usecc = xml.attribute("Convert").as_bool();
			ccdest = this->GetCasingByName(xml.attribute("To").as_string());
			ccsrc = this->GetCasingByName(xml.attribute("From").as_string());
		});
		auto& args = m_Config->AddClass("Command", [this](const pugi::xml_node xml, std::shared_ptr<void> & obj){
			// store the command to the object pointer so we can access it again
			auto command = AddCommand(xml.attribute("Name").as_string(), xml.attribute("ID").as_ullong());
			obj = command;
		}).AddClass("Args");
		args.AddClass("Arg", [this,&types](const pugi::xml_node xml, std::shared_ptr<void> & obj){
			// retrieve the object poiter as a SCR command we know it to be
			auto& command = *std::static_pointer_cast<SCRambl::Command>(obj);
			if (auto type = types.GetType(xml.attribute("Type").as_string())) {
				command.AddArg(type->Extend(), xml.attribute("Out").as_bool());
			}
			else {
				std::string name = xml.attribute("Type").as_string();
				name.size();
			}
		});
	}
}