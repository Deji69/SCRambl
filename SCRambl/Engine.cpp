#include "stdafx.h"
#include "Engine.h"

using namespace SCRambl;

Engine::Engine() : HaveTask(false), m_Commands(*this)
{
	m_Commands.Init();
}

const TaskSystem::Task<EngineEvent> & Engine::Run()
{
	if(CurrentTask == std::end(Tasks)) CurrentTask = std::begin(Tasks);
	auto & it = CurrentTask;
	
	if (it != std::end(Tasks))
	{
		auto task = it->second;
		
		while (task->IsTaskFinished())
		{
			++it;
			if (it == std::end(Tasks)) return *this;
			task = it->second;
		}

		task->RunTask();
	}

	return *this;
}