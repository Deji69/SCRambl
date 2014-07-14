/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <list>
#include <memory>
#include "Builder.h"
#include "Tasks.h"
#include "Formatter.h"
#include "Reporting.h"

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

	public:
		Engine();
		virtual ~Engine()
		{
			/*if (!Tasks.empty())
			{
				for (auto task : Tasks)
				{
					delete task.second;
				}

				Tasks.clear();
			}*/
		}

		/*\
		 - Set, override or cancel the override of a string formatter
		\*/
		template<typename T, typename F>
		inline void SetFormatter(F &func)
		{
			Formatters[&typeid(T)] = std::make_shared<Formatter<T>>(func);
		}

		/*\
		 - String format a SCRambl type
		\*/
		template<typename T>
		inline std::string Format(const T& param) const
		{
			if (!Formatters.empty())
			{
				auto it = Formatters.find(&typeid(T));
				if (it != Formatters.end())
				{
					return it->second->Qualify<T>()(param);
				}
			}


			return "";
		}

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