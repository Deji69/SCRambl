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
#include "utils.h"
#include "Builder.h"
#include "Tasks.h"
#include "Formatter.h"
#include "Reporting.h"
#include "Configuration.h"
#include "Commands.h"

namespace SCRambl
{
	enum class EngineEvent
	{

	};

	class Engine : protected TaskSystem::Task<EngineEvent>
	{
		//using TaskEntry = std::pair<int, TypeSystem::Task>;
		using TaskMap = std::map<int, std::shared_ptr<TaskSystem::ITask>>;
		using FormatMap = std::map<const std::type_info*, std::shared_ptr<IFormatter>>;
		using ConfigMap = std::map < std::string, std::shared_ptr<Configuration> >;

		// Configuration
		ConfigMap				m_Config;

		// Tasks
		TaskMap					Tasks;
		TaskMap::iterator		CurrentTask;
		//TaskSystem::Task<TaskSystem::Event>::State	LastTaskState;
		bool					HaveTask;

		// Message formatting
		FormatMap				Formatters;

		inline void Init() {
			CurrentTask = std::begin(Tasks);
		}

		Commands				m_Commands;

	public:
		Engine();
		virtual ~Engine()
		{
		}

		/*\
		 * Engine::AddConfig - Returns shared Configuration element
		\*/
		std::shared_ptr<Configuration> AddConfig(const std::string & name)
		{
			if (name.empty()) return nullptr;
			if (m_Config.find(name) != m_Config.end()) return nullptr;
			auto config = std::make_shared<Configuration>(name);
			m_Config.emplace(name, config);
			return config;
		}

		/*\
		 * Engine::LoadConfigFile
		\*/
		bool LoadConfigFile(const std::string & path)
		{
			pugi::xml_document xml;
			auto status = xml.load_file(widen(path).c_str());
			if (status) {
				// find our element
				auto scrambl = xml.child("SCRambl");
				if (scrambl) {
					// load configurations
					if (m_Config.size()) {
						for (auto node : scrambl.children()) {
							if (*node.name()) {
								// find configuration
								auto it = m_Config.find(node.name());
								if (it != m_Config.end()) {
									// load from node
									it->second->LoadXML(node);
								}
							}
						}
					}
				}
				return true;
			}
			else {
				
			}
			return false;
		}

		/*\
		 * Engine::SetFormatter<> - Set, override or cancel the override of a string formatter
		\*/
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
			if (!HaveTask)
			{
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