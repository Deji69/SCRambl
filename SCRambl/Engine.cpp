#include "stdafx.h"
#include "Engine.h"

using namespace SCRambl;

Build Engine::InitBuild(Script& script, std::vector<std::string> files) {
	Build build;
	auto config = m_Builder.GetConfig();
	for (auto path : files) {
		auto file = m_Builder.LoadFile(build, path);
		if (!file) {
			BREAK();
		}
	}

	
}

bool Engine::LoadXML(const std::string& path) {
	XML xml(path);
	if (xml) {
		// load configurations
		if (m_Config.size()) {
			for (auto node : xml.Children()) {
				if (!node.Name().empty()) {
					// find configuration
					auto it = m_Config.find(node.Name());
					if (it != m_Config.end()) {
						// load from node
						it->second->LoadXML(node);
					}
				}
			}
		}
		return true;
	}
	return false;
}

Engine::Engine():
	HaveTask(false),
	// in that order...
	m_Builder(*this),
	m_Constants(*this),
	m_Types(*this),
	m_Commands(*this)
{
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
			if (it == std::end(Tasks)) {
				m_State = finished;
				return *this;
			}
			task = it->second;
		}

		task->RunTask();
	}

	return *this;
}