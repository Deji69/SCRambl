#include "stdafx.h"
#include "Types.h"
#include "Commands.h"
#include "Engine.h"

namespace SCRambl
{
	Commands::Commands(Engine & eng) : m_Engine(eng)
	{
		auto& types = m_Engine.GetTypes();

		m_Config = m_Engine.AddConfig("Commands");
		auto& args = m_Config->AddClass("Command", [this](const pugi::xml_node xml, std::shared_ptr<void> & obj){
			// store the command to the object pointer so we can access it again
			auto command = AddCommand(xml.attribute("Name").as_string(), xml.attribute("ID").as_ullong());
			obj = command;
		}).AddClass("Args");
			args.AddClass("Arg", [this,&types](const pugi::xml_node xml, std::shared_ptr<void> & obj){
				// retrieve the object poiter as a SCR command we know it to be
				auto& command = *std::static_pointer_cast<SCRambl::SCR::Command>(obj);
				if (auto type = types.GetType(xml.attribute("Type").as_string())) {
					
				}
			});
	}
	void Commands::Init() {
		//m_Config = m_Engine.AddConfig("Commands");
		//m_Config->AddClass("Command", "Name", "Opcode");
	}
}