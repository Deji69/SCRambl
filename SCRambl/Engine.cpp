#include "stdafx.h"
#include "Engine.h"

using namespace SCRambl;

Engine::Engine()
{
	CurrentTask = Tasks.end();
}

const Task & Engine::Run() const
{
	auto it = CurrentTask != Tasks.end() ? CurrentTask : Tasks.begin();
	
	if (it != Tasks.end())
	{
		auto task = it->first;
		
		while (it->first->GetState() == Task::finished)
		{
			++it;
			if (it == Tasks.end()) return *this;
		}

		return task->Run();
	}
	return *this;
}