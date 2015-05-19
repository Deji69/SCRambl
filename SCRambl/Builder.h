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

	enum class BuildEvent {

	};

	struct BuildVariable {
		XMLValue Value;
		BuildVariable() = default;
		BuildVariable(XMLValue v) : Value(v) { }
	};

	class BuildEnvironment
	{
		Engine& m_Engine;
		std::map<std::string, BuildVariable> m_Variables;

	public:
		BuildEnvironment(Engine&);

		template<typename T>
		BuildVariable& Set(std::string id, const T& v) {
			auto it = m_Variables.find(id);
			if (it != std::end(m_Variables))
				it->second.Value = m_Engine.Format(v);
			else
				it = m_Variables.emplace(id, v);
			return it;
		}
		BuildVariable& Get(std::string id) {
			return m_Variables[id];
		}
		
		inline std::string Val(XMLValue v) {
			std::string val, raw = v.AsString();
			if (raw.size() <= 3) val = raw;
			else {
				for (size_t i = 0; i < raw.size(); ++i) {
					if (raw[i] == '(' && raw[i + 1] == '$') {
						auto end = raw.find_first_of(')', i + 2);
						if (end != raw.npos) {
							size_t arr = 0;
							auto lbr = raw.find_first_of('[', i);
							if (lbr != raw.npos) {
								end = lbr;
								auto rbr = raw.find_first_of(']', lbr);
								if (rbr != raw.npos) {
									arr = std::stoul(raw.substr(lbr, rbr - lbr));
								}
								else lbr = rbr;
							}

							if (lbr != raw.npos) {
								val += Val(Get(raw.substr(i, lbr - i)).Value.AsList()[arr]);
							}
							else {
								val += Val(Get(raw.substr(i, end - i)).Value);
							}

							i = end;
							continue;
						}
					}
					val += raw[i];
				}
			}
			return val;
		}
	};

	class Build : public TaskSystem::Task<BuildEvent>
	{
		friend class Builder;

		struct BuildScript {
			std::string ID;
			std::string Output;
			std::vector<std::string> Inputs;

			BuildScript() = default;
			BuildScript(std::string id, std::string output) : ID(id), Output(output)
			{ }
		};
		struct BuildInput {
			std::string ScriptType;
			std::string Value;
			Scripts::File::Shared Input;

			BuildInput() = default;
			BuildInput(std::string type, std::string value) : ScriptType(type), Value(value)
			{ }
		};

		Engine& m_Engine;
		BuildEnvironment m_Env;
		BuildConfig::Shared m_Config;
		Script m_Script;
		std::vector<BuildScript> m_BuildScripts;
		std::vector<BuildInput> m_BuildInputs;
		std::vector<std::string> m_Files;

		// Tasks
		using TaskMap = std::map<int, std::shared_ptr<TaskSystem::ITask>>;
		TaskMap m_Tasks;
		TaskMap::iterator m_CurrentTask;
		bool m_HaveTask;

		void Init();

	public:
		using Shared = std::shared_ptr<Build>;

		Build(Engine&, BuildConfig::Shared);

		Scripts::File::Shared AddInput(std::string);

		inline Script& GetScript() { return m_Script; }
		inline const Script& GetScript() const { return m_Script; }

		template<typename T, typename ID, typename... Params>
		const std::shared_ptr<T> AddTask(ID id, Params&&... prms) {
			auto task = std::shared_ptr<T>(new T(m_Engine, std::forward<Params>(prms)...));
			m_Tasks.emplace(id, task);
			if (!m_HaveTask) {
				Init();
				m_HaveTask = true;
			}
			return task;
		}
		template<typename ID>
		bool RemoveTask(ID id) {
			if (!m_Tasks.empty()) {
				auto it = std::find(std::begin(m_Tasks), std::end(m_Tasks), id);
				if (it != std::end(Tasks)) {
					delete it->second;
					m_Tasks.erase(it);

					if (m_Tasks.empty()) {
						m_HaveTask = false;
						m_CurrentTask = std::end(m_Tasks);
					}
					return true;
				}
			}
			return false;
		}

		template<typename T>
		inline T & GetCurrentTask()					{ return reinterpret_cast<T&>(*CurrentTask->second); }
		inline int GetCurrentTaskID() const			{ return std::ref(m_CurrentTask->first); }
		inline size_t GetNumTasks() const			{ return m_Tasks.size(); }
		inline void ClearTasks()					{ m_Tasks.clear(); }

		const TaskSystem::Task<BuildEvent> & Run();

	protected:
		bool IsTaskFinished() override				{ return m_CurrentTask == std::end(m_Tasks); }
		void ResetTask() override					{ Init(); }
		void RunTask() override						{ Run(); }
	};

	class Builder
	{
		Engine& m_Engine;
		XMLConfiguration::Shared m_Configuration;
		XMLConfig& m_Config;
		std::unordered_map<std::string, std::shared_ptr<BuildConfig>> m_BuildConfigurations;
		std::shared_ptr<BuildConfig> m_BuildConfig;

	public:
		Builder(Engine&);
		
		Scripts::File::Shared LoadFile(Build::Shared, std::string);
		bool LoadDefinitions(Build::Shared);

		bool LoadScriptFile(std::string, Script&);
		bool SetConfig(std::string) const { return true; }
		std::shared_ptr<BuildConfig> GetConfig() const { return m_BuildConfig; }
	};
}