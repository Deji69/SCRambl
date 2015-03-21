#include "stdafx.h"
#include "config.h"

GlobalConfiguration* g_config;

GlobalConfiguration&	GlobalConfig() {
	static GlobalConfiguration config;
	g_config = &config;
	return config;
}

ConfigBase::ConfigBase(std::string name, Configurable* configurable, bool isTemp) : m_Name(name), m_Configurable(configurable), m_IsTemp(isTemp)
{
	if (!m_IsTemp) GlobalConfig().Add(this);
}
ConfigBase::~ConfigBase()
{
	if (!m_IsTemp) GlobalConfig().Remove(this);
}

void GlobalConfiguration::Add(ConfigBase* config)
{
	m_Map[config->GetName()] = config;
}
bool GlobalConfiguration::Remove(ConfigBase* config)
{
	auto it = m_Map.find(config->GetName());
	if (it != m_Map.end())
	{
		m_Map.erase(it);
		return true;
	}
	return false;
}
void GlobalConfiguration::Save()
{
	for (auto it : m_Map) {
		it.second->Save();
	}
}
void GlobalConfiguration::Load()
{
	for (auto it : m_Map) {
		it.second->Load();
	}
}