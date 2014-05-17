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
		using TaskEntry = std::pair<Task*, int>;
		std::list<TaskEntry>							Tasks;
		std::list<TaskEntry>::iterator					CurrentTask;

		void Init()
		{
			/*if (Tasks.size())
			{
				for (auto task : Tasks)
				{
					delete task.first;
				}

				Tasks.clear();
			}*/

			CurrentTask = Tasks.begin();
		}

	public:
		Engine();

		template<class ID, class T, class ... Params>
		inline void AddTask(ID id, Params &... prms)	{ Tasks.push_back(TaskEntry(new T(*this, prms...), id)); }

		template<class T>
		inline T & GetCurrentTask()					{ return CurrentTask->first; }
		inline int GetCurrentTaskID() const			{ return CurrentTask->second; }
		inline size_t GetNumTasks() const			{ return Tasks.size(); }
		inline void ClearTasks()					{ Tasks.clear(); }

		const Task & Run() const;

	protected:
		bool IsTaskFinished() override				{ return CurrentTask == Tasks.end(); }
		void ResetTask() override					{ Init(); }
		void RunTask() override						{ Run(); }
	};
}