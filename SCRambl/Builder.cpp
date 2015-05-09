#include "stdafx.h"
#include "utils.h"
#include "Builder.h"
#include "Engine.h"

namespace SCRambl
{
	namespace BuildSystem
	{
		void Builder::Run()
		{
			switch (m_State)
			{
			case init:
				Init();
				break;
			case preprocess:
			case parse:
			case compile:
			case link:
				break;
			default:
				break;
			}
		}

		void Builder::Init()
		{

		}

		bool Builder::LoadScriptFile(const std::string& path, Script& script) {
			if (auto config = GetConfig()) {
				if (auto file_type = config->GetFileType(path)) {
					switch (file_type->GetType())
					{
					case FileType::compiled:
					case FileType::merged:
					case FileType::parsed:
						//script.LoadFile(path);
						break;
					case FileType::source:
						//script.LoadFile(path);
						break;
					default: return false;
					}
					return true;
				}
			}
			return false;
		}
		bool Builder::SetConfig(const std::string& name) {
			auto it = name.empty() ? m_BuildConfigurations.end() : m_BuildConfigurations.find(name);
			if (it != m_BuildConfigurations.end()) {
				m_BuildConfig = it->second;
				return true;
			}
			else if (!m_BuildConfigurations.empty()) {
				m_BuildConfig = m_BuildConfigurations.begin()->second;
				return true;
			}
			return false;
		}
		std::shared_ptr<BuildConfig> Builder::GetConfig() const {
			return m_BuildConfig;
		}

		void BuildConfig::OutputFile(const std::string& name) const {
			pugi::xml_document doc;
			auto node = doc.append_child("SCRambl");
			node.append_attribute("Version") = "1.0";
			node.append_attribute("FileVersion") = "0.0.0.0";
			
			auto proj_node = node.append_child("Project");
			proj_node.append_attribute("Name") = name.c_str();
			node = proj_node.append_child("Configuration");
			node.append_attribute("ID") = "";

			doc.save_file((name+".xml").c_str());
		}
		std::shared_ptr<FileType> BuildConfig::AddFileType(FileType::Type type, std::string name, std::string ext) {
			if (!name.empty() && !ext.empty()) {
				auto ft = std::make_shared<FileType>(type, name, ext);
				m_FileTypes.emplace(ext, ft);
				return ft;
			}
			return nullptr;
		}
		std::shared_ptr<FileType> BuildConfig::GetFileType(const std::string& path) {
			auto it = m_FileTypes.find(GetFilePathExtension(path));
			return it != m_FileTypes.end() ? it->second : nullptr;
		}
		bool BuildConfig::AddIncludePath(const std::string& path) {
			if (!path.empty()) {
				m_IncludePaths.emplace_back(path);
				return true;
			}
			return false;
		}
		bool BuildConfig::AddDefaultLoad(const std::string& path) {
			if (!path.empty()) {
				m_LoadDefaults.emplace_back(GetDefinitionPath() + path);
				return true;
			}
			return false;
		}
		bool BuildConfig::AddDefaultLoadLocally(const std::string& path) {
			if (!path.empty()) {
				m_LoadDefaults.emplace_back(path);
				return true;
			}
			return false;
		}
		bool BuildConfig::AddDefinitionPath(const std::string& path) {
			if (!path.empty()) {
				if (m_UsedDefinitionPaths.find(path) != m_UsedDefinitionPaths.end()) {
					m_DefinitionPaths.emplace_back(path);
					m_UsedDefinitionPaths.emplace(path);
				}
				return true;
			}
			return false;
		}
		bool BuildConfig::SetDefinitionPath(std::string path) {
			if (!path.empty()) {
				m_DefinitionPath = path;
				return true;
			}
			return false;
		}
		bool BuildConfig::SetLibraryPath(const std::string& path) {
			if (!path.empty()) {
				m_LibraryPath = path;
				return true;
			}
			return false;
		}
		const std::string& BuildConfig::GetDefinitionPath() const {
			return m_DefinitionPath;
		}
		const std::string& BuildConfig::GetLibraryPath() const {
			return m_LibraryPath;
		}

		FileType::FileType(Type type, std::string name, std::string ext) : m_Type(type),
			m_Name(name), m_Extension(ext)
		{ }

		BuildConfig::BuildConfig(std::string id, std::string name) : m_ID(id), m_Name(name)
		{ }
		BuildConfig::BuildConfig(std::string id, std::string name, Configuration::Config& config) : m_ID(id), m_Name(name)
		{
			// <DefinitionPath>s
			auto& defpath = config.AddClass("DefinitionPath", [](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				auto ptr = std::static_pointer_cast<BuildConfig>(obj);
				if (auto attr = vec.attribute("Path")) {
					if (ptr->AddDefinitionPath(attr.as_string()))
						ptr->SetDefinitionPath(attr.as_string());
				}
				ptr->SetDefinitionPath("");
			});

			// <DefinitionPath>'s <Definition>s
			defpath.AddClass("Definition", [](const pugi::xml_node vec, std::shared_ptr<void>& obj){
				auto ptr = std::static_pointer_cast<BuildConfig>(obj);
				std::string path = vec.text().as_string();
				if (!path.empty()) {
					ptr->AddDefaultLoad(path);
				}
			});

			// <Definition>s
			config.AddClass("Definition", [](const pugi::xml_node vec, std::shared_ptr<void>& obj){
				auto ptr = std::static_pointer_cast<BuildConfig>(obj);
				std::string path = vec.text().as_string();
				if (!path.empty()) {
					ptr->AddDefaultLoadLocally(path);
				}
			});
			// <LibraryPath>
			config.AddClass("LibraryPath", [](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				auto ptr = std::static_pointer_cast<BuildConfig>(obj);
				ptr->SetLibraryPath(vec.text().as_string());
			});
			// <IncludePath> <Path>s
			config.AddClass("IncludePaths", [](const pugi::xml_node vec, std::shared_ptr<void> & obj){
			}).AddClass("Path", [](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				auto ptr = std::static_pointer_cast<BuildConfig>(obj);
				if (!vec.text().empty()) {
					ptr->AddIncludePath(vec.text().as_string());
				}
			});
			// <Script>
			auto& script = config.AddClass("Script", [](const pugi::xml_node vec, std::shared_ptr<void>& obj){
			});
			script.AddClass("Input", [](const pugi::xml_node vec, std::shared_ptr<void>& obj) {

			});

			/*config.AddClass("FileType", [](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				auto ptr = std::static_pointer_cast<BuildConfig>(obj);
				auto attr = vec.attribute("Name");
				if (!attr.empty()) {
					auto name = attr.as_string();
					attr = vec.attribute("FormatExt");
					if (!attr.empty()) {
						auto ext = attr.as_string();
						attr = vec.attribute("Type");
						if (!attr.empty()) {
							auto type = attr.as_string();
							FileType::Type ftype;
							if (FileType::GetTypeByName(type, ftype))
							{
								obj = ptr->AddFileType(ftype, name, ext);
								return;
							}
						}
					}
				}

				obj = nullptr;
			});*/
		}
		
		Builder::Builder(Engine& engine) : m_Engine(engine), m_Config(engine.AddConfig("BuildConfig")),
			m_ConfigurationConfig(m_Config->AddClass("Build", [this](const pugi::xml_node vec, std::shared_ptr<void> & obj){
				auto attr = vec.attribute("ID");
				if (!attr.empty()) {
					std::string id = attr.as_string();
					attr = vec.attribute("Name");
					if (!attr.empty()) {
						std::string name = attr.as_string();
						auto ptr = std::make_shared<BuildConfig>(id, name, m_ConfigurationConfig);
						m_BuildConfigurations.emplace(id, ptr);
						obj = ptr;
					}
				}
			}))
		{ }
	}
}