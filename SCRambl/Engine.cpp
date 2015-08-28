#include "stdafx.h"
#include "Standard.h"
#include "Engine.h"
#include "Preprocessor.h"
#include "Parser.h"
#include "Compiler.h"
#include "Linker.h"

using namespace SCRambl;

enum BuildTask {
	preprocessor, parser, compiler, linker, finished
};

bool Engine::BuildScript(Build* build) {
	auto state = build->Run().GetState();
	return state != TaskSystem::Task::finished;
}
Build* Engine::InitBuild(std::vector<std::string> files) {
	auto config = m_Builder.GetConfig();
	auto build = new Build(*this, config);
	m_Builder.LoadDefinitions(build);

	for (auto path : files) {
		auto file = m_Builder.LoadFile(build, path);
		if (!file) {
			BREAK();
		}
	}

	auto add_task_events = [](BuildTask id, TaskSystem::Task* task) {
		
	};
	auto preprocessor_task = build->AddTask<Preprocessing::Task>(preprocessor, build);
	auto parser_task = build->AddTask<Parsing::Task>(parser, build);
	auto compiler_task = build->AddTask<Compiling::Task>(compiler, build);
	auto linker_task = build->AddTask<Linking::Task>(linker, build);
	add_task_events(preprocessor, preprocessor_task);
	add_task_events(parser, parser_task);
	add_task_events(compiler, compiler_task);
	add_task_events(linker, linker_task);
	auto get_task_name = [](const Build* build){
		switch (build->GetCurrentTaskID()) {
		case preprocessor:
			return "Preprocessor";
		case parser:
			return "Parser";
		case compiler:
			return "Compiler";
		case linker:
			return "Linker:";
		}
		return "";
	};
	build->AddEventHandler<task_event>([&get_task_name, build](const task_event& event){
		std::cout << get_task_name(build) << ": event `" << event.Name() << "`\n";
		return true;
	});
	build->AddEventHandler<build_event>([&get_task_name, build](const build_event& event){
		std::cout << get_task_name(build) << ": event `" << event.Name() << "`\n";
		return true;
	});
	build->AddEventHandler<error_event>([&get_task_name, build](const error_event& event){
		std::cerr << get_task_name(build) << ": ERROR:";
		auto error = event.Error.Get<Parsing::Error>();
		using Parsing::Error;
		switch (error) {
		case Error::invalid_identifier:
			break;
		default:
			for (auto& str : event.Params)
				std::cerr << " " << str;
			break;
		}
		std::cerr << "\n";
		return true;
	});
	return build;
}
void Engine::FreeBuild(Build* build) {
	if (build) {
		delete build;
		build = nullptr;
	}
}
BuildConfig* Engine::GetBuildConfig() const {
	return m_Builder.GetConfig();
}
bool Engine::SetBuildConfig(const std::string& name) {
	return m_Builder.SetConfig(name);
}
XMLConfiguration* Engine::AddConfig(const std::string& name) {
	if (name.empty()) return nullptr;
	if (m_Config.find(name) != m_Config.end()) return nullptr;
	auto pr = m_Config.emplace(name, name);
	return pr.second ? &pr.first->second : nullptr;
}
bool Engine::LoadFile(const std::string& path, Script& script) {
	return GetFilePathExtension(path) == "xml" ? LoadXML(path) : m_Builder.LoadScriptFile(path, script);
}
bool Engine::LoadBuildFile(const std::string& path, const std::string& buildConfig) {
	if (LoadXML(path)) {
		m_Builder.SetConfig(buildConfig);
		if (auto config = m_Builder.GetConfig()) {
			return true;
		}
	}
	return false;
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
						it->second.LoadXML(node);
					}
				}
			}
		}
		return true;
	}
	return false;
}
Engine::Engine() : m_Builder(*this)
{ }
Engine::~Engine()
{ }