#include "stdafx.h"
#include "Engine.h"

using namespace SCRambl;

Engine::Engine() : HaveTask(false)
{
	//CurrentTask = Tasks.end();
}

const TaskBase & Engine::Run()
{
	if(CurrentTask == std::end(Tasks)) CurrentTask = std::begin(Tasks);
	auto & it = CurrentTask;
	
	if (it != std::end(Tasks))
	{
		auto task = it->second;
		
		while (task->GetState() == TaskBase::finished)
		{
			++it;
			if (it == std::end(Tasks)) return *this;
			task = it->second;
		}

		LastTaskState = task->Run().GetState();
	}

	return *this;
}