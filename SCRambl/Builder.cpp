#include "stdafx.h"
#include "utils.h"
#include "Builder.h"
#include "Engine.h"

namespace SCRambl
{
	/* Build */
	void Build::LoadDefinitions() {
		auto loads = m_Config->GetDefinitions();
		size_t defs_not_loaded = loads.size();
		for (auto& defpath : m_Config->GetDefinitionPaths()) {
			for (auto& def : defpath.Definitions) {
				if (!LoadXML(defpath.Path + def))
					++defs_not_loaded;
			}

			for (auto it = loads.begin(); it != loads.end(); ++it) {
				if (LoadXML(defpath.Path + *it)) {
					it = loads.erase(it);
					--defs_not_loaded;
				}
			}
		}
		//defs_not_loaded = 0;
	}
	bool Build::LoadXML(std::string path) {
		XML xml(path);
		if (xml) {
			// load configurations
			if (m_ConfigMap.size()) {
				for (auto node : xml.Children()) {
					if (!node.Name().empty()) {
						// find configuration
						auto it = m_ConfigMap.find(node.Name());
						if (it != m_ConfigMap.end()) {
							// load from node
							it->second->LoadXML(node);
						}
					}
				}
			}
			return true;
		}
		return false;
	}
	void Build::Init() {
		m_CurrentTask = std::begin(m_Tasks);

		LoadDefinitions();

		for (auto& scr : m_Config->GetScripts()) {
			m_BuildScripts.emplace_back(scr.first, m_Env.Val(scr.second->Name) + m_Env.Val(scr.second->Ext));
		}
	}
	Scripts::File::Shared Build::AddInput(std::string input) {
		// TODO:
		return m_Script.OpenFile(input);
	}
	std::shared_ptr<XMLConfiguration> Build::AddConfig(const std::string & name)
	{
		if (name.empty()) return nullptr;
		auto it = m_ConfigMap.find(name);
		if (it != m_ConfigMap.end()) return it->second;
		auto config = std::make_shared<XMLConfiguration>(name);
		m_ConfigMap.emplace(name, config);
		return config;
	}
	const TaskSystem::Task<BuildEvent> & Build::Run() {
		if (m_CurrentTask == std::end(m_Tasks))
			Init();
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
	{
		m_Commands.Init(*this);
		m_Types.Init(*this);
		m_Constants.Init(*this);
	}

	/* BuildEnvironment */
	BuildEnvironment::BuildEnvironment(Engine& engine) : m_Engine(engine) { }

	/* BuildConfig */
	ScriptConfig::Shared BuildConfig::AddScript(std::string id, XMLValue name, XMLValue ext) {
		auto ptr = std::make_shared<ScriptConfig>(name, ext);
		m_Scripts.emplace(id, ptr);			// TODO: ?
		return ptr;
	}
	void BuildConfig::AddDefinitionPath(std::string path, const std::vector<std::string>& defs) {
		auto& defpath = AddDefPath(path);
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
	template<ParseObjectConfig::ActionType TAction>
	void AddEnvironmentHandler(XMLConfig& config, std::string str) {
		config.AddClass(str, [](const XMLNode base, std::shared_ptr<void>& obj) {
			if (auto ptr = std::static_pointer_cast<ParseObjectConfig>(obj)) {
				if (auto attr = base.GetAttribute("Var")) {
					ParseObjectConfig::Action action;
					action.Type = TAction;
					action.Var = attr.GetValue();
					ptr->Actions.emplace_back(action);
				}
			}
		});
	}
	void AddEnvironmentXMLHandlers(XMLConfig& config) {
		AddEnvironmentHandler<ParseObjectConfig::ActionType::Set>(config, "Set");
		AddEnvironmentHandler<ParseObjectConfig::ActionType::Inc>(config, "Inc");
		AddEnvironmentHandler<ParseObjectConfig::ActionType::Dec>(config, "Dec");
	}
	BuildConfig::BuildConfig(std::string id, std::string name, XMLConfig& config) : m_ID(id), m_Name(name) {
		// <DefinitionPath>...</DefinitionPath>
		auto& defpath = config.AddClass("DefinitionPath", [](const XMLNode base, std::shared_ptr<void>& obj) {
			auto ptr = std::static_pointer_cast<BuildConfig>(obj);
			if (auto attr = base.GetAttribute("Path")) {
				std::vector<std::string> defs;
				for (auto child : base.Children()) {
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
			auto& parsecmd = parse.AddClass("Command", [](const XMLNode base, std::shared_ptr<void>& obj){
				auto ptr = std::static_pointer_cast<BuildConfig>(obj);
				if (auto attr = base.GetAttribute("Name")) {
					ParseObjectConfig::Shared config = std::make_shared<ParseObjectConfig>();
					config->Name = attr.GetValue();
					config->Required = base.GetAttribute("Required").GetValue();
					config->Type = base.GetAttribute("Type").GetValue();
					ptr->m_ParseCommandNames.emplace_back(attr.GetValue(), config);
					obj = config;
				}
			});
			AddEnvironmentXMLHandlers(parsecmd);
			// </Command>
			// <Variable>
			auto& parsevar = parse.AddClass("Variable", [](const XMLNode base, std::shared_ptr<void>& obj){
				auto ptr = std::static_pointer_cast<BuildConfig>(obj);
				if (auto attr = base.GetAttribute("Name")) {
					ParseObjectConfig::Shared config = std::make_shared<ParseObjectConfig>();
					config->Name = attr.GetValue();
					config->Required = base.GetAttribute("Required").GetValue();
					config->Type = base.GetAttribute("Type").GetValue();
					ptr->m_ParseVariableNames.emplace_back(attr.GetValue(), config);
					obj = config;
				}
			});
			AddEnvironmentXMLHandlers(parsevar);
			// </Variable>
			// <Label>
			auto& parselabel = parse.AddClass("Label", [](const XMLNode base, std::shared_ptr<void>& obj){
				auto ptr = std::static_pointer_cast<BuildConfig>(obj);
				if (auto attr = base.GetAttribute("Name")) {
					ParseObjectConfig::Shared config = std::make_shared<ParseObjectConfig>();
					config->Name = attr.GetValue();
					config->Required = base.GetAttribute("Required").GetValue();
					config->Type = base.GetAttribute("Type").GetValue();
					ptr->m_ParseLabelNames.emplace_back(attr.GetValue(), config);
					obj = config;
				}
			});
			AddEnvironmentXMLHandlers(parselabel);
			// </Label>
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

	/* build_xmlvalue_less */
	bool build_xmlvalue_less::operator()(const XMLValue& lhs, const XMLValue& rhs) const {
		return env.Val(lhs) < env.Val(rhs);
	}
}