#include "stdafx.h"
#include "Engine.h"

using namespace SCRambl;

Engine::Engine() : HaveTask(false)
{
	//CurrentTask = Tasks.end();
}

const Task & Engine::Run() const
{
	auto it = CurrentTask != std::end(Tasks) ? CurrentTask : std::begin(Tasks);
	
	if (it != std::end(Tasks))
	{
		auto task = it->second;
		
		while (task->GetState() == Task::finished)
		{
			++it;
			if (it == std::end(Tasks))
				return *this;
		}

		return task->Run();
	}
	return *this;
}