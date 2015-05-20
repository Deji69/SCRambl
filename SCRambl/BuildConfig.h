/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <unordered_map>
#include "Tasks.h"
#include "Configuration.h"
#include "Scripts.h"

namespace SCRambl
{
	struct build_xmlvalue_less {
		class BuildEnvironment& env;
		bool operator()(const XMLValue& lhs, const XMLValue& rhs) const;
	};
	struct BuildDefinitionPath
	{
		std::string Path;
		std::vector<std::string> Definitions;

		//BuildDefinitionPath() = default;
		BuildDefinitionPath(std::string path);
	};

	struct InputConfig {
		enum eType {
			File
		};

		using Shared = std::shared_ptr<InputConfig>;
		InputConfig::InputConfig(XMLValue val, eType type) : Value(val), Type(type)
		{ }

		XMLValue Value;
		eType Type;
	};
	struct ScriptConfig {
		using Shared = std::shared_ptr<ScriptConfig>;
		ScriptConfig::ScriptConfig(XMLValue name, XMLValue ext) : Name(name), Ext(ext)
		{ }

		XMLValue Name;
		XMLValue Ext;
		std::vector<InputConfig> Inputs;
	};
	
	struct ParseObjectConfig {
		using Shared = std::shared_ptr<ParseObjectConfig>;
		enum class ActionType {
			Set, Inc, Dec, 
		};

		struct Action {
			ActionType Type;
			XMLValue Var;

			Action() = default;
			Action(ActionType type, XMLValue var) : Type(type), Var(var)
			{ }
		};

		XMLValue Name;
		XMLValue Type;
		XMLValue Required;
		std::vector<Action> Actions;
	};

	class BuildConfig
	{
		friend class Build;

		XMLConfiguration::Shared m_Config;

		// attributes
		std::string m_ID;
		std::string m_Name;

		// paths
		std::vector<BuildDefinitionPath> m_DefinitionPaths;
		std::map<const std::string, size_t> m_DefinitionPathMap;
		std::vector<std::string> m_Definitions;

		//
		std::map<std::string, ScriptConfig::Shared> m_Scripts;
		std::vector<std::pair<XMLValue, ParseObjectConfig::Shared>> m_ParseCommandNames;
		std::vector<std::pair<XMLValue, ParseObjectConfig::Shared>> m_ParseVariableNames;
		std::vector<std::pair<XMLValue, ParseObjectConfig::Shared>> m_ParseLabelNames;

		BuildDefinitionPath& AddDefPath(std::string);
		size_t GetDefinitionPathID(std::string);				// returns -1 on failure

		inline auto GetScripts() const->const decltype(m_Scripts)&  {
			return m_Scripts;
		}

	public:
		using Shared = std::shared_ptr<BuildConfig>;

		//BuildConfig::BuildConfig(std::string id, std::string name);
		BuildConfig::BuildConfig(std::string id, std::string name, XMLConfig& config);

		ScriptConfig::Shared AddScript(std::string id, XMLValue name, XMLValue ext = "");
		void AddDefinitionPath(std::string);
		void AddDefinitionPath(std::string, const std::vector<std::string>&);

		size_t GetNumDefinitionPaths() const { return m_DefinitionPaths.size(); }
		size_t GetNumDefaultLoads() const { return 0; }
		std::string GetDefaultLoad(size_t i = 0) const { return ""; }
		std::string GetDefinitionPath(size_t i) const { return m_DefinitionPaths[i].Path; }

		const std::vector<std::string>& GetDefinitions() const { return m_Definitions; }
		const std::vector<BuildDefinitionPath>& GetDefinitionPaths() const { return m_DefinitionPaths; }
	};
}