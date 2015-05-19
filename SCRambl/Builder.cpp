#include "stdafx.h"
#include "utils.h"
#include "Builder.h"
#include "Engine.h"

namespace SCRambl
{
	/* Build */
	void Build::Init() {
		m_CurrentTask = std::begin(m_Tasks);

		for (auto& scr : m_Config->GetScripts()) {
			m_BuildScripts.emplace_back(scr.first, m_Env.Val(scr.second->Name) + m_Env.Val(scr.second->Ext));
		}
	}
	Scripts::File::Shared Build::AddInput(std::string input) {
		// TODO:
		return m_Script.OpenFile(input);
	}
	const TaskSystem::Task<BuildEvent> & Build::Run() {
		if (m_CurrentTask == std::end(m_Tasks)) Init();
		auto& it = m_CurrentTask;

		if (it != std::end(m_Tasks)) {
			auto task = it->second;
			while (task->IsTaskFinished()) {
				if (++it == std::end(m_Tasks)) {
					return *this;
				}
				task = it->second;
			}
			task->RunTask();
		}
		return *this;
	}
	Build::Build(Engine& engine, BuildConfig::Shared config) : m_Env(engine), m_Engine(engine),
		m_Config(config), m_CurrentTask(std::end(m_Tasks))
	{ }

	/* BuildEnvironment */
	BuildEnvironment::BuildEnvironment(Engine& engine) : m_Engine(engine) { }

	/* BuildConfig */
	ScriptConfig::Shared BuildConfig::AddScript(std::string id, XMLValue name, XMLValue ext) {
		auto ptr = std::make_shared<ScriptConfig>(name, ext);
		m_Scripts.emplace(id, ptr);			// TODO: ?
		return ptr;
	}
	void BuildConfig::AddDefinitionPath(std::string path, const std::vector<std::string>& defs) {
		auto defpath = AddDefPath(path);
		std::remove_copy_if(defs.begin(), defs.end(), std::back_inserter(defpath.Definitions), VectorContainPred<std::string>(defpath.Definitions));
	}
	void BuildConfig::AddDefinitionPath(std::string path) {
		AddDefPath(path);
	}
	// TODO: implement path stripping so all paths are formatted the same (no chance of duplicate paths with different strings)
	BuildDefinitionPath& BuildConfig::AddDefPath(std::string path) {
		auto id = GetDefinitionPathID(path);
		if (id == -1) {
			id = m_DefinitionPaths.size();
			m_DefinitionPathMap.emplace(path, id);
			m_DefinitionPaths.emplace_back(path);
		}
		return m_DefinitionPaths[id];
	}
	size_t BuildConfig::GetDefinitionPathID(std::string path) {
		auto it = m_DefinitionPathMap.find(path);
		return it != m_DefinitionPathMap.end() ? it->second : -1;
	}
	BuildConfig::BuildConfig(std::string id, std::string name, XMLConfig& config) : m_ID(id), m_Name(name) {
		// <DefinitionPath>...</DefinitionPath>
		auto& defpath = config.AddClass("DefinitionPath", [](const XMLNode base, std::shared_ptr<void>& obj) {
			auto ptr = std::static_pointer_cast<BuildConfig>(obj);
			if (auto attr = base.GetAttribute("Path")) {
				std::vector<std::string> defs;
				for (auto child : base) {
					if (child.Name() == "Definition") {
						auto str = child.GetValue().AsString();
						if (!str.empty()) {
							defs.emplace_back(str);
						}
					}
				}

				if (defs.empty())
					ptr->AddDefinitionPath(attr.GetValue().AsString());
				else
					ptr->AddDefinitionPath(attr.GetValue().AsString(), defs);
			}
		});

		// <Parse>
		auto& parse = config.AddClass("Parse");
			// <Command>
			parse.AddClass("Command", [](const XMLNode base, std::shared_ptr<void>& obj){

			});
		// </Parse>

		// <Script>
		auto& script = config.AddClass("Script", [](const XMLNode base, std::shared_ptr<void>& obj){
			auto ptr = std::static_pointer_cast<BuildConfig>(obj);
			if (auto attr = base.GetAttribute("ID")) {
				obj = ptr->AddScript(attr.GetValue().AsString(), base.GetAttribute("Name").GetValue(), base.GetAttribute("Ext").GetValue());
			}
			else obj = nullptr;
		});
			// <Input>
			auto& input = script.AddClass("Input");
				// <File>
				input.AddClass("File", [](const XMLNode base, std::shared_ptr<void>& obj){
					auto ptr = std::static_pointer_cast<ScriptConfig>(obj);
					ptr->Inputs.emplace_back(base.GetValue(), InputConfig::File);
				});
				// </File>
			// </Input>
		// </Script>
	}
	//BuildConfig::BuildConfig(std::string id, std::string name) : m_ID(id), m_Name(name)
	//{ }

	/* Builder */
	bool Builder::LoadDefinitions(Build::Shared build) {
		auto definitions = GetConfig()->GetDefinitions();
		for (auto path : GetConfig()->GetDefinitionPaths()) {
			for (auto def : path.Definitions) {
				m_Engine.LoadDefinition(path.Path + def);
			}

			for (auto it = definitions.begin(); it != definitions.end(); ++it) {
				if (m_Engine.LoadDefinition(*it))
					it = definitions.erase(it);
			}
		}
		return true;
	}
	Scripts::File::Shared Builder::LoadFile(Build::Shared build, std::string path) {
		return build->AddInput(path);
	}
	bool Builder::LoadScriptFile(std::string path, Script& script) {
		return true;
	}
	Builder::Builder(Engine& engine) : m_Engine(engine), m_Configuration(engine.AddConfig("BuildConfig")),
		m_Config(m_Configuration->AddClass("Build", [this](const XMLNode path, std::shared_ptr<void>& obj){
			// get attributes
			if (auto attr = path.GetAttribute("ID")) {
				auto id = attr.GetValue().AsString();
				if (attr = path.GetAttribute("Name")) {
					auto name = attr.GetValue().AsString();

					// add build configuration
					auto ptr = std::make_shared<BuildConfig>(id, name, m_Config);
					if (!m_BuildConfig || path.GetAttribute("Default").GetValue().AsBool())
						m_BuildConfig = ptr;
					m_BuildConfigurations.emplace(id, ptr);

					// set for the BuildConfig XML loader
					obj = ptr;
				}
			}
		}))
	{ }

	/* DefinitionPath */
	BuildDefinitionPath::BuildDefinitionPath(std::string path) : Path(path)
	{ }
}