#include "stdafx.h"
#include "Types.h"
#include "Engine.h"

namespace SCRambl
{
	Types::Types(Engine & eng) : m_Engine(eng)
	{
		m_Config = m_Engine.AddConfig("Types");
		auto& type = m_Config->AddClass("Type", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
			// store the command to the object pointer so we can access it again
			unsigned long id = 0;
			if(auto attr = vec.attribute("ID"))
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