#include "stdafx.h"
#include "Configuration.h"
#include <memory>

namespace SCRambl
{
	/* XMLObject */
	void* XMLObject::LoadXML(XMLNode node, void* theptr) {
		if (m_Func) m_Func(node, theptr);
		return theptr;
	}

	/* XMLConfig */
	void XMLConfig::LoadChildXML(XMLRange root, void* ptr) {
		if (m_Objects.empty()) return;
		for (auto node : root) {
			auto it = m_Objects.find(node.Name());
			if (it != m_Objects.end()) {
				auto& obj = it->second;
				auto new_ptr = obj.LoadXML(node, ptr);
				obj.LoadChildXML(node.Children(), new_ptr);
				continue;
			}
		}
	}
	XMLConfig* XMLConfig::AddClass(const std::string& name) {
		auto pr = m_Objects.emplace(name, XMLObject());
		return pr.second ? &pr.first->second : nullptr;
	}

	/* XMLConfiguration */
	void XMLConfiguration::LoadXML(XMLNode main_node) {
		if (!m_Objects.empty()) {
			for (auto node : main_node) {
				auto it = m_Objects.find(node.Name());
				if (it != m_Objects.end()) {
					auto& obj = it->second;
					obj.LoadChildXML(node.Children(), obj.LoadXML(node));
					continue;
				}
			}
		}
	}
	XMLConfiguration::XMLConfiguration(std::string name) : m_Name(name)
	{ }
}