#include "stdafx.h"
#include "Configuration.h"
#include <memory>

namespace SCRambl
{
	/* XMLObject */
	std::shared_ptr<void> XMLObject::LoadXML(XMLNode node, std::shared_ptr<void> theptr = nullptr) {
		if (m_Func) {
			auto & func = *static_cast<std::function<void(const XMLNode, std::shared_ptr<void> &)>*>(m_Func.get());
			func(node, theptr);
		}
		return theptr;
	}

	/* XMLConfig */
	void XMLConfig::LoadChildXML(XMLRange root, std::shared_ptr<void> ptr) {
		if (m_Objects.empty()) return;
		for (auto node : root) {
			auto it = m_Objects.find(node.Name());
			if (it != m_Objects.end())
			{
				auto obj = it->second;
				auto new_ptr = obj->LoadXML(node, ptr);
				obj->LoadChildXML(node.Children(), new_ptr);
				continue;
			}
		}
	}

	XMLConfig XMLConfig::AddClass(const std::string& name) {
		auto obj = std::make_shared<XMLObject>();
		m_Objects.emplace(name, obj);
		return *obj;
	}

	/* XMLConfiguration */
	void XMLConfiguration::LoadXML(XMLNode main_node) {
		if (!m_Groups.empty()) {
			for (auto node : main_node) {
				auto it = m_Groups.find(node.Name());
				if (it != m_Groups.end()) {
					auto obj = it->second;
					obj->LoadChildXML(node.Children(), obj->LoadXML(node));
					continue;
				}
			}
		}
	}
	XMLConfiguration::XMLConfiguration(std::string name) : m_Name(name)
	{ }
}