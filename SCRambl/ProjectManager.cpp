#include "stdafx.h"
#include "utils.h"
#include "utils\xml.h"
#include "ProjectManager.h"

namespace SCRambl{

	//CProject		*	s_pProject;

	void Project::LoadFile(const char* path) {
		pugi::xml_document xml;
		auto err = xml.load_file(path);
		if (err.status == pugi::xml_parse_status::status_ok) {

		}
		throw;
	}
	void Project::SaveFile(const char* path) const {
		pugi::xml_document doc;
		auto node = doc.append_child("SCRambl");
		node.append_attribute("Version") = "1.0";
		node.append_attribute("FileVersion") = "0.0.0.0";

		auto proj_node = node.append_child("Project");
		proj_node.append_attribute("Name") = GetName().c_str();
		node = proj_node.append_child("Configuration");
		//node.append_attribute("ID") = GetConfig()->GetID().c_str();
		
		auto files_node = proj_node.append_child("Files");

		doc.save_file((GetName() + ".xml").c_str());
	}
}