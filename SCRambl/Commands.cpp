#include "stdafx.h"
#include "Commands.h"
#include "Engine.h"

namespace SCRambl {
	Commands::Commands(Engine & eng) : m_Engine(eng)
	{
		m_Config = m_Engine.AddConfig("Commands");
		auto& args = m_Config->AddClass("Command", [this](const std::vector<pugi::xml_attribute> & vec, std::shared_ptr<void> & obj){
			// store the command to the object pointer so we can access it again
			obj = AddCommand(vec[0].as_string(), vec[1].as_ullong());
		}, "Name", "ID").AddClass("Args");
		args.AddClass("Arg", [this](const std::vector<pugi::xml_attribute> & vec, std::shared_ptr<void> & obj){
			// retrieve the object poiter as a SCR command we know it to be
			auto& command = *std::static_pointer_cast<SCRambl::SCR::Command>(obj);
			std::cout << "Type: " << vec[0].as_string() << "\n";
			std::cout << command.GetName();
		}, "Type");
	}
	void Commands::Init() {
		//m_Config = m_Engine.AddConfig("Commands");
		//m_Config->AddClass("Command", "Name", "Opcode");
	}
}