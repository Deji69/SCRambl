#include "stdafx.h"
#include "Commands.h"
#include "Engine.h"

namespace SCRambl {
	Commands::Commands(Engine & eng) : m_Engine(eng)
	{
		m_Config = m_Engine.AddConfig("Commands");
		m_Config->AddClass("Command", [this](const std::vector<pugi::xml_attribute> & vec){
			auto command = AddCommand(vec[0].as_string(), vec[1].as_ullong());
		}, "Name", "ID");
	}
	void Commands::Init() {
		//m_Config = m_Engine.AddConfig("Commands");
		//m_Config->AddClass("Command", "Name", "Opcode");
	}
}