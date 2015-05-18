/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <list>
#include <memory>
#include <map>
#include <iostream>
#include "utils.h"
#include "Builder.h"
#include "Tasks.h"
#include "Formatter.h"
#include "Reporting.h"
#include "Configuration.h"
#include "Commands.h"
#include "Types.h"
#include "Constants.h"
#include "Scripts.h"

namespace SCRambl
{
	enum class EngineEvent {
		ConfigurationError,
	};

	class Engine : public TaskSystem::Task<EngineEvent>
	{
		//using TaskEntry = std::pair<int, TypeSystem::Task>;
		using TaskMap = std::map < int, std::shared_ptr<TaskSystem::ITask> >;
		using FormatMap = std::map < const std::type_info*, std::shared_ptr<IFormatter> >;
		using ConfigMap = std::map < std::string, std::shared_ptr<XMLConfiguration> >;

		// Configuration
		ConfigMap				m_Config;

		// Tasks
		TaskMap					Tasks;
		TaskMap::iterator		CurrentTask;
		//TaskSystem::Task<TaskSystem::Event>::State	LastTaskState;
		bool					HaveTask;

		// BuildSystem
		Builder	m_Builder;

		// Message formatting
		FormatMap				Formatters;

		inline void Init() {
			CurrentTask = std::begin(Tasks);
		}

		Constants				m_Constants;
		Commands				m_Commands;
		Types::Types			m_Types;

		bool LoadXML(const std::string & path);

	public:
		Engine();
		virtual ~Engine()
		{ }

		Build::Shared InitBuild(Script&, std::vector<std::string> files);
		bool BuildScript(Build::Shared);

		// Get SCR Commands
		inline Commands & GetCommands()			{ return m_Commands; }
		// Get SCR Types
		inline Types::Types & GetTypes()		{ return m_Types; }

		/*\
		 * Engine::GetBuildConfig
		\*/
		inline std::shared_ptr<BuildConfig> GetBuildConfig() const {
			return m_Builder.GetConfig();
		}

		/*\
		 * Engine::SetBuildConfig
		\*/
		inline bool SetBuildConfig(std::string name) {
			return m_Builder.SetConfig(name);
		}

		/*\
		 * Engine::AddConfig - Returns shared Configuration element
		\*/
		std::shared_ptr<XMLConfiguration> AddConfig(const std::string & name)
		{
			if (name.empty()) return nullptr;
			if (m_Config.find(name) != m_Config.end()) return nullptr;
			auto config = std::make_shared<XMLConfiguration>(name);
			m_Config.emplace(name, config);
			return config;
		}

		std::shared_ptr<Configuration> AddConfiguration(const std::string & name) {
			return std::make_shared<Configuration>(name);
		}

		/*/ Engine::AddConfig /*/
		void AddConfig(std::shared_ptr<Configuration> config) {
			//m_Config.emplace(config->GetName(), config);
		}

		/*/ Engine::LoadFile /*/
		bool LoadFile(const std::string & path, Script& script) {
			return GetFilePathExtension(path) == "xml" ? LoadXML(path) : m_Builder.LoadScriptFile(path, script);
		}

		/*/ Engine::LoadConfigFile - Loads a build file (e.g. build.xml) and applies the buildConfig /*/
		bool LoadBuildFile(const std::string& path, const std::string& buildConfig = "") {
			if (LoadXML(path)) {
				m_Builder.SetConfig(buildConfig);
				if (auto config = m_Builder.GetConfig()) {
					return true;
				}
			}
			return false;
		}

		/*/ Engine::LoadDefinition /*/
		bool LoadDefinition(std::string filename, std::string * full_path_out = nullptr)
		{
			if(LoadXML(filename)) {
				return true;
			}
			return false;
#if 0
			auto l = m_Builder.GetConfig()->GetNumDefinitionPaths();
			for (size_t i = 0; i < l; ++i) {
				std::string path = m_Builder.GetConfig()->GetDefinitionPath(i) + filename;
				if (LoadXML(path)) {
					if (full_path_out) *full_path_out = path;
					return true;
				}
			}
			return false;
#endif
		}

		/*/ Engine::SetFormatter<> - Set, override or cancel the override of a string formatter /*/
		template<typename T, typename F>
		void SetFormatter(F &func)
		{
			Formatters[&typeid(T)] = std::make_shared<Formatter<T>>(func);
		}

		/*\
		 * Engine::AddTask<> - Add task to the running engine
		\*/
		template<typename T, typename ID, typename... Params>
		const std::shared_ptr<T> AddTask(ID id, Params&&... prms)
		{
			auto task = std::shared_ptr<T>(new T(*this, std::forward<Params>(prms)...));
			Tasks.emplace(id, task);
			if (!HaveTask) {
				Init();
				HaveTask = true;
			}
			return task;
		}

		/*\
		 * Engine::RemoveTask<> - Remove task from the running engine
		\*/
		template<typename ID>
		bool RemoveTask(ID id)
		{
			if (!Tasks.empty())
			{
				auto it = std::find(std::begin(Tasks), std::end(Tasks), id);
				if (it != std::end(Tasks))
				{
					delete it->second;
					Tasks.erase(it);

					if (Tasks.empty())
					{
						HaveTask = false;
						CurrentTask = std::end(Tasks);
					}
					return true;
				}
			}
			return false;
		}

		/*\
		 * Engine::Format<> - String format a SCRambl type
		\*/
		template<typename T>
		inline std::string Format(const T& param) const
		{
			if (!Formatters.empty())
			{
				auto k = &typeid(T);
				auto it = Formatters.find(&typeid(T));
				if (it != Formatters.end()) {
					return it->second->Qualify<T>()(param);
				}
			}
			return "";
		}

		// specialisations for easy non-SCRambl types
		//template<> inline std::string Format(std::string & param) const				{ return param; }
		template<> inline std::string Format(const std::string& param) const		{ return param; }
		template<> inline std::string Format(const int& param) const				{ return std::to_string(param); }
		template<> inline std::string Format(const unsigned int& param) const		{ return std::to_string(param); }
		template<> inline std::string Format(const long& param) const				{ return std::to_string(param); }
		template<> inline std::string Format(const unsigned long& param) const		{ return std::to_string(param); }
		template<> inline std::string Format(const long long& param) const			{ return std::to_string(param); }
		template<> inline std::string Format(const unsigned long long& param) const { return std::to_string(param); }
		template<> inline std::string Format(const float& param) const				{ return std::to_string(param); }
		template<> inline std::string Format(const double& param) const				{ return std::to_string(param); }
		template<> inline std::string Format(const long double& param) const		{ return std::to_string(param); }

		/*\
		 * Engine::Format<T, T, ...> - String format multiple types
		\*/
		template<typename First, typename... Args>
		void Format(std::vector<std::string> & out, First&& first, Args&&... args)
		{
			// do one
			out.push_back(Format(first));
			// continue
			Format(out, args...);
		}

		/*\
		 * Engine::Format<T> - String format multiple types
		\*/
		template<typename Last>
		inline void Format(std::vector<std::string> & out, Last&& last)
		{
			// finale
			out.push_back(Format(std::forward<Last>(last)));
		}

	public:
		template<typename T>
		inline T & GetCurrentTask()					{ return reinterpret_cast<T&>(*CurrentTask->second); }
		inline int GetCurrentTaskID() const			{ return std::ref(CurrentTask->first); }
		inline size_t GetNumTasks() const			{ return Tasks.size(); }
		inline void ClearTasks()					{ Tasks.clear(); }

		const TaskSystem::Task<EngineEvent> & Run();

	protected:
		bool IsTaskFinished() override				{ return CurrentTask == std::end(Tasks); }
		void ResetTask() override					{ Init(); }
		void RunTask() override						{ Run(); }
	};
}