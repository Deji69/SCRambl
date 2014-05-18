/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Builder.h"
#include "Tasks.h"
#include <list>

namespace SCRambl
{
	class Engine : protected Task
	{
		using TaskEntry = std::pair<int, Task*>;
		//std::list<TaskEntry>							Tasks;
		//std::list<TaskEntry>::iterator					CurrentTask;
		std::map<int, Task*>							Tasks;
		std::map<int, Task*>::iterator					CurrentTask;
		bool											HaveTask;

		void Init()
		{
			CurrentTask = std::begin(Tasks);
		}

	public:
		Engine();
		virtual ~Engine()
		{
			if (!Tasks.empty())
			{
				for (auto task : Tasks)
				{
					delete task.second;
				}

				Tasks.clear();
			}
		}

		template<class ID, class T, class ... Params>
		void AddTask(ID id, Params &... prms)
		{
			Tasks.emplace(id, new T(*this, prms...));
			if (!HaveTask)
			{
				Init();
				HaveTask = true;
			}
		}

		template<class T>
		inline T & GetCurrentTask()					{ return *reinterpret_cast<T*>((*CurrentTask).second); }
		inline int GetCurrentTaskID() const			{ return (*CurrentTask).first; }
		inline size_t GetNumTasks() const			{ return Tasks.size(); }
		inline void ClearTasks()					{ Tasks.clear(); }

		const Task & Run() const;

	protected:
		bool IsTaskFinished() override				{ return CurrentTask == std::end(Tasks); }
		void ResetTask() override					{ Init(); }
		void RunTask() override						{ Run(); }
	};
}