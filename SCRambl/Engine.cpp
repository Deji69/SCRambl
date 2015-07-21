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
	return state != TaskSystem::Task<BuildEvent>::finished;
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

	auto preprocessor_task = build->AddTask<Preprocessor::Task>(preprocessor, build);
	auto parser_task = build->AddTask<Parser::Task>(parser, build);
	auto compiler_task = build->AddTask<Compiler::Task>(compiler, build);
	auto linker_task = build->AddTask<Linker::Task>(linker, build);
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

Basic::Error::Error(const Preprocessor::Error& err) : m_Type(preprocessor),
	m_Payload(std::make_unique<Info<Preprocessor::Error>>(err))
{ }
Basic::Error::Error(const Parser::Error& err) : m_Type(parser),
	m_Payload(std::make_unique<Info<Parser::Error>>(err))
{ }
Basic::Error::Error(Error&& o) : m_Payload(std::move(o.m_Payload))
{ }
Basic::Error& Basic::Error::operator=(Error&& o) {
	if (this != &o)
		m_Payload = std::move(o.m_Payload);
	return *this;
}