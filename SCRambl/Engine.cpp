#include "stdafx.h"
#include "Engine.h"
#include "Preprocessor.h"
#include "Parser.h"
#include "Compiler.h"

using namespace SCRambl;

enum BuildTask {
	preprocessor, parser, compiler, linker, finished
};

bool Engine::BuildScript(Build::Shared build) {
	auto state = build->Run().GetState();

	
	return true;
}

Build::Shared Engine::InitBuild(Script& script, std::vector<std::string> files) {
	auto config = m_Builder.GetConfig();
	auto build = std::make_shared<Build>(*this, config);
	m_Builder.LoadDefinitions(build);

	for (auto path : files) {
		auto file = m_Builder.LoadFile(build, path);
		if (!file) {
			BREAK();
		}
	}

	auto preprocessor_task = build->AddTask<Preprocessor::Task>(preprocessor, std::ref(build->GetScript()));
	auto parser_task = build->AddTask<Parser::Task>(parser, std::ref(build->GetScript()));
	auto compiler_task = build->AddTask<Compiler::Task>(compiler, std::ref(build->GetScript()));
	return build;
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
const TaskSystem::Task<EngineEvent> & Engine::Run() {
	if(CurrentTask == std::end(Tasks)) CurrentTask = std::begin(Tasks);
	auto & it = CurrentTask;
	
	if (it != std::end(Tasks)) {
		auto task = it->second;
		
		while (task->IsTaskFinished()) {
			if (++it == std::end(Tasks)) {
				m_State = finished;
				return *this;
			}
			task = it->second;
		}

		task->RunTask();
	}
	return *this;
}
Engine::Engine() :
HaveTask(false),
// in that order...
m_Builder(*this), m_Constants(*this), m_Types(*this), m_Commands(*this)
{ }
