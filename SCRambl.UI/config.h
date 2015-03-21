#ifndef CONFIG_H
#define CONFIG_H
#include <QSettings>
#include "pugixml.hpp"

class Configurable
{
public:
	virtual void onConfigUpdate() = 0;
};

class GlobalConfiguration;

GlobalConfiguration& GlobalConfig();

class ConfigBase
{
	friend class GlobalConfiguration;
	std::string				m_Name;
	bool					m_IsTemp;
	Configurable		*	m_Configurable;

	inline const std::string&		GetName()			{ return m_Name; }

public:
	ConfigBase(std::string name, Configurable* = nullptr, bool = false);
	virtual ~ConfigBase();

	void Save() {
		if (!m_IsTemp) {
			QSettings settings;
			settings.beginGroup(m_Name.c_str());
			Save(settings);
			if (m_Configurable) m_Configurable->onConfigUpdate();
		}
	}
	void Load() {
		if (!m_IsTemp) {
			QSettings settings;
			settings.beginGroup(m_Name.c_str());
			Load(settings);
			if (m_Configurable) m_Configurable->onConfigUpdate();
		}
	}

protected:
	virtual void Load(const QSettings&) = 0;
	virtual void Save(QSettings&) = 0;
};

class GlobalConfiguration
{
public:
	GlobalConfiguration() : m_QSettings("GTAG Modding", "SCRambl UI")
	{
		QCoreApplication::setOrganizationName("GTAG Modding");
		QCoreApplication::setOrganizationDomain("gtag.gtagaming.com");
		QCoreApplication::setApplicationName("SCRambl UI");
	}
	void Add(ConfigBase* config);
	bool Remove(ConfigBase* config);
	void Load();
	void Save();
	
	template<typename T = ConfigBase>
	inline T* Get(const std::string& name) const
	{
		auto it = m_Map.find(name);
		return it != m_Map.end() ? static_cast<T*>(it->second) : nullptr;
	}
	
	inline QSettings& Settings()					{ return m_QSettings; }
	inline const QSettings& Settings() const		{ return m_QSettings; }

private:
	QSettings								m_QSettings;
	std::map<std::string, ConfigBase*>		m_Map;
};

#endif