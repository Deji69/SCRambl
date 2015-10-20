#include "stdafx.h"
#include "BuildConfig.h"
#include "Builder.h"

using namespace SCRambl;

/* BuildConfig */
ScriptConfig* BuildConfig::AddScript(std::string id, XMLValue name, XMLValue ext) {
	auto pr = m_Scripts.emplace(id, ScriptConfig(name, ext));			// TODO: ?
	return pr.second ? &pr.first->second : nullptr;
}
void BuildConfig::AddDefinitionPath(std::string path, const std::vector<std::string>& defs) {
	auto& defpath = AddDefPath(path);
	std::remove_copy_if(defs.begin(), defs.end(), std::back_inserter(defpath.Definitions), VectorContainPred<std::string>(defpath.Definitions));
}
void BuildConfig::AddDefinitionPath(std::string path) {
	AddDefPath(path);
}
BuildDefinitionPath& BuildConfig::AddDefPath(std::string path) {
	// TODO: implement path stripping so all paths are formatted the same (no chance of duplicate paths with different strings)
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
void AddEnvironmentHandler(XMLConfig* config, std::string str) {
	config->AddClass(str, [](const XMLNode base, void*& obj) {
		if (auto ptr = static_cast<ParseObjectConfig*>(obj)) {
			if (auto attr = base.GetAttribute("Var")) {
				ParseObjectConfig::Action action;
				action.Type = TAction;
				action.Var = attr.GetValue();
				ptr->Actions.emplace_back(action);
			}
		}
	});
}
void AddEnvironmentXMLHandlers(XMLConfig* config) {
	AddEnvironmentHandler<ParseObjectConfig::ActionType::Set>(config, "Set");
	AddEnvironmentHandler<ParseObjectConfig::ActionType::Inc>(config, "Inc");
	AddEnvironmentHandler<ParseObjectConfig::ActionType::Dec>(config, "Dec");
}
ParseObjectConfig* BuildConfig::AddParseObjectConfig(XMLValue name, XMLValue required, XMLValue type) {
	m_ObjectConfigs.emplace_back();
	auto& obj = m_ObjectConfigs.back();
	obj.Name = name;
	obj.Required = required;
	obj.Type = type;
	return &obj;
}
BuildConfig::BuildConfig(std::string id, std::string name, XMLConfig* config) : m_ID(id), m_Name(name) {
	// <DefinitionPath>...</DefinitionPath>
	auto defpath = config->AddClass("DefinitionPath", [](const XMLNode base, void*& obj) {
		auto ptr = static_cast<BuildConfig*>(obj);
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
	if (auto parse = config->AddClass("Parse")) {
		// <Command>
		auto parsecmd = parse->AddClass("Command", [](const XMLNode base, void*& obj){
			auto ptr = static_cast<BuildConfig*>(obj);
			if (auto attr = base.GetAttribute("Name")) {
				auto config = ptr->AddParseObjectConfig(attr.GetValue(), base.GetAttribute("Required").GetValue(), base.GetAttribute("Type").GetValue());
				ptr->m_ParseCommandNames.emplace_back(attr.GetValue(), config);
				obj = config;
			}
		});
		AddEnvironmentXMLHandlers(parsecmd);
		// </Command>
		// <Variable>
		auto parsevar = parse->AddClass("Variable", [](const XMLNode base, void*& obj){
			auto ptr = static_cast<BuildConfig*>(obj);
			if (auto attr = base.GetAttribute("Name")) {
				auto config = ptr->AddParseObjectConfig(attr.GetValue(), base.GetAttribute("Required").GetValue(), base.GetAttribute("Type").GetValue());
				ptr->m_ParseVariableNames.emplace_back(attr.GetValue(), config);
				obj = config;
			}
		});
		AddEnvironmentXMLHandlers(parsevar);
		// </Variable>
		// <Label>
		auto parselabel = parse->AddClass("Label", [](const XMLNode base, void*& obj){
			auto ptr = static_cast<BuildConfig*>(obj);
			if (auto attr = base.GetAttribute("Name")) {
				auto config = ptr->AddParseObjectConfig(attr.GetValue(), base.GetAttribute("Required").GetValue(), base.GetAttribute("Type").GetValue());
				ptr->m_ParseLabelNames.emplace_back(attr.GetValue(), config);
				obj = config;
			}
		});
		AddEnvironmentXMLHandlers(parselabel);
		// </Label>
	} // </Parse>

	// <Optimisation>
	if (auto optimisation = config->AddClass("Optimisation")) {
		// <Level>
		auto level = optimisation->AddClass("Level", [](const XMLNode base, void*& obj){
			auto ptr = static_cast<BuildConfig*>(obj);
			auto val = base.GetValue().AsString("NONE");
			ptr->Optimisation().SetOptimisationLevel(val);
		});
		// </Level>
	} // </Optimisation>

	// <Script>
	if (auto script = config->AddClass("Script", [](const XMLNode base, void*& obj){
		auto ptr = static_cast<BuildConfig*>(obj);
		if (auto attr = base.GetAttribute("ID")) {
			obj = ptr->AddScript(attr.GetValue().AsString(), base.GetAttribute("Name").GetValue(), base.GetAttribute("Ext").GetValue());
		}
		else obj = nullptr;
	})) {
		// <Input>
		if (auto input = script->AddClass("Input")) {
			// <File>
			input->AddClass("File", [](const XMLNode base, void*& obj){
				auto ptr = static_cast<ScriptConfig*>(obj);
				ptr->Inputs.emplace_back(base.GetValue(), InputConfig::File);
			});
			// </File>
		} // </Input>
	} // </Script>
}

/* DefinitionPath */
BuildDefinitionPath::BuildDefinitionPath(std::string path) : Path(path)
{ }

/* build_xmlvalue_less */
bool isLessCaseInsensitive(const std::string& lhs, const std::string& rhs) {
	auto lit = lhs.begin();
	for (auto c : rhs) {
		if (lit == lhs.end() || std::tolower(*lit) < std::tolower(c)) return true;
		++lit;
	}
	return false;
}
bool build_xmlvalue_less::operator()(const XMLValue& lhs, const XMLValue& rhs) const {
	return caseSensitive ? env.Val(lhs).AsString() < env.Val(rhs).AsString() : isLessCaseInsensitive(env.Val(lhs).AsString(), env.Val(rhs).AsString());
}