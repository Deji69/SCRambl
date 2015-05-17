#include "stdafx.h"
#include "Delimiters.h"
#include "Engine.h"

namespace SCRambl
{
	Delimiters::Delimiters(Engine& eng) {
		m_config = eng.AddConfiguration("Delimiters");
	}

	void Delimiters::AddDelimiterType(std::string name) {
		/*m_config->AddClass(name, [](const pugi::xml_node xml, ){

		});*/
	}
}