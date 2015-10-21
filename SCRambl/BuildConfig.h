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
		bool caseSensitive = true;
		bool operator()(const XMLValue& lhs, const XMLValue& rhs) const;
	};
	struct BuildDefinitionPath
	{
		std::string Path;
		std::vector<std::string> Definitions;
		BuildDefinitionPath(std::string);
	};

	struct InputConfig {
		enum eType {
			File
		};

		InputConfig::InputConfig(XMLValue val, eType type) : Value(val), Type(type)
		{ }

		XMLValue Value;
		eType Type;
	};
	struct ScriptConfig {
		ScriptConfig::ScriptConfig(XMLValue name, XMLValue ext) : Name(name), Ext(ext)
		{ }

		XMLValue Name;
		XMLValue Ext;
		std::vector<InputConfig> Inputs;
	};
	struct OptimisationConfig {
		enum Level {
			NONE = 0,
			LOW,
			CHAIN_CONST_OPS,
			MEDIUM = CHAIN_CONST_OPS,
			HIGH,
			ALL = LOW | MEDIUM | HIGH
		};

		static Level GetOptimisationLevelByName(const std::string name) {
			static const std::unordered_map<std::string, Level> map = {
				{ "NONE", NONE }, { "ALL", ALL },
				{ "LOW", LOW }, { "MEDIUM", MEDIUM }, { "HIGH", HIGH },
				{ "CHAIN_CONST_OPS", CHAIN_CONST_OPS }
			};
			uint32_t level = NONE;
			enum { op_nop, op_and, op_or } op = op_nop;
			for (size_t i = 0; i < name.size(); ++i) {
				size_t j = i;
				while (std::isalpha(name[i]) || name[i] == '_')
					if (++i == name.size())
						break;
				if (j == i) {
					if (name[i] == '|')
						op = op_or;
					else if (name[i] == '&')
						op = op_and;
				}
				else {
					auto str = toupper(name.substr(j, i));
					auto it = map.find(str);
					if (it != map.end()) {
						if (op == op_and)
							level &= it->second;
						else if (op == op_or)
							level |= it->second;
						else
							level = it->second;
					}
					op = op_nop;
				}
			}
			return static_cast<Level>(level);
		}

		inline OptimisationConfig& SetOptimisationLevel(Level level) {
			m_Level = level;
			return *this;
		}
		inline OptimisationConfig& SetOptimisationLevel(const std::string name) {
			return SetOptimisationLevel(GetOptimisationLevelByName(name));
		}
		inline bool CheckLevel(Level level) { return (m_Level & level) != 0; }

		Level m_Level = ALL;
	};
	struct ParseObjectConfig {
		enum class ActionType {
			Clear, Set, Inc, Dec, Add, Sub, Mul, Div, Mod, And, Or, Xor, Shl, Shr, Not
		};

		struct Action {
			ActionType Type;
			XMLValue Var;

			Action() = default;
			Action(ActionType type, XMLValue var) : Type(type), Var(var)
			{ }
		};

		using ActionVec = std::vector<Action>;
		XMLValue Name;
		XMLValue Type;
		XMLValue Required;
		ActionVec Actions;
	};

	class BuildConfig
	{
		friend class Build;
		using ParseNameVec = std::vector<std::pair<XMLValue, ParseObjectConfig*>>;
		using ParseConfigVec = std::vector<ParseObjectConfig>;
		using ScriptConfigMap = std::map<std::string, ScriptConfig>;

	public:
		BuildConfig::BuildConfig(std::string id, std::string name, XMLConfig* config);

		// Add script type configuration
		ScriptConfig* AddScript(std::string id, XMLValue name, XMLValue ext = "");
		// Add definition path
		void AddDefinitionPath(std::string);
		// Add definition path + default definition files
		void AddDefinitionPath(std::string, const std::vector<std::string>&);
		// Add configuration for a parse object
		ParseObjectConfig* AddParseObjectConfig(XMLValue name, XMLValue required, XMLValue type);

		inline size_t GetNumDefinitionPaths() const { return m_DefinitionPaths.size(); }
		inline size_t GetNumDefaultLoads() const {
			size_t n = 0;
			for (auto& defpath : m_DefinitionPaths) {
				n += defpath.Definitions.size();
			}
			return n + m_Definitions.size();
		}
		//inline std::string GetDefaultLoad(size_t i = 0) const { return ""; }
		inline std::string GetDefinitionPath(size_t i) const { return m_DefinitionPaths[i].Path; }
		inline const std::vector<std::string>& GetDefinitions() const { return m_Definitions; }
		inline const std::vector<BuildDefinitionPath>& GetDefinitionPaths() const { return m_DefinitionPaths; }

		inline OptimisationConfig& Optimisation() { return m_OptimisationConfig; }

	protected:
		const ParseNameVec& GetParseCommands() const { return m_ParseCommandNames; }
		const ParseNameVec& GetParseVariables() const { return m_ParseVariableNames; }
		const ParseNameVec& GetParseLabels() const { return m_ParseLabelNames; }

	private:
		BuildDefinitionPath& AddDefPath(std::string);
		size_t GetDefinitionPathID(std::string);				// returns -1 on failure

		inline const ScriptConfigMap& GetScripts() const  { return m_Scripts; }

		XMLConfiguration* m_Config;

		// attributes
		std::string m_ID;
		std::string m_Name;

		// paths
		std::vector<BuildDefinitionPath> m_DefinitionPaths;
		std::map<const std::string, size_t> m_DefinitionPathMap;
		std::vector<std::string> m_Definitions;

		//
		ScriptConfigMap m_Scripts;
		ParseNameVec m_ParseCommandNames;
		ParseNameVec m_ParseVariableNames;
		ParseNameVec m_ParseLabelNames;
		ParseConfigVec m_ObjectConfigs;
		OptimisationConfig m_OptimisationConfig;
	};
}