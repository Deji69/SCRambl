#include "stdafx.h"
#include "parser.h"
#include "editor.h"

Processor& GetProcessor()
{
	static Processor engine;
	return engine;
}

Processor::Processor() : m_Config(this),
	m_Running(nullptr)
{
	m_Script = new SCRambl::Script();
	m_SCRamblEngine.LoadConfigFile("config\\build.xml");
	
	m_Preprocessor = m_SCRamblEngine.AddTask<SCRambl::Preprocessor::Task>(ProcessorTask::Preprocessor, std::ref(*m_Script));
	m_Parser = m_SCRamblEngine.AddTask<SCRambl::Parser::Task>(ProcessorTask::Parser, std::ref(*m_Script));
	m_Preprocessor->AddEventHandler<SCRambl::Preprocessor::Event::Finish>([](){
		return true;
	});

	m_CodeIsOld = true;
}

Processor::~Processor()
{
	delete m_Script;
}

KeywordType Processor::getKeywordType(const QString &kw)
{
	SCRambl::Commands::Vector vec;
	if (m_SCRamblEngine.GetCommands().FindCommands(kw.toStdString(), vec) > 0)
		return KeywordType::command;
	return KeywordType::none;
}

void Processor::onConfigUpdate()
{
	m_SCRamblEngine.LoadConfigFile(m_Config.BuildConfigFilePath.toUtf8().constData());
	m_SCRamblEngine.SetBuildConfig(m_Config.BuildConfig.toUtf8().constData());
	m_SCRamblEngine.LoadDefinition("constants");
	m_SCRamblEngine.LoadDefinition("types");
	m_SCRamblEngine.LoadDefinition("commands");
}

void Processor::errorString(QString err)
{
}

void Processor::parseFinished()
{
	if (!m_Script->GetCode().IsEmpty() && m_CodeIsOld) {
		auto& tokens = m_Script->GetTokens();
		for (auto token : tokens)
		{
			auto type = token->GetToken()->GetType<SCRambl::Tokens::Type>();
			auto symbol = token->GetSymbol();
			auto pos = token->GetPosition();
			switch (type)
			{
			case SCRambl::Tokens::Type::Directive:
			case SCRambl::Tokens::Type::Command:
			case SCRambl::Tokens::Type::CommandDecl:
			case SCRambl::Tokens::Type::CommandCall:
			case SCRambl::Tokens::Type::CommandOverload:
				break;
			case SCRambl::Tokens::Type::Identifier: {
				auto& info = token->GetToken()->Get<SCRambl::Tokens::Identifier::Info<>>();
				auto range = info.GetValue<0>();
				auto name = range.Format();
				info = info;
				break;
			}
			case SCRambl::Tokens::Type::Label:
			case SCRambl::Tokens::Type::LabelRef:
			case SCRambl::Tokens::Type::Number:
			case SCRambl::Tokens::Type::Operator:
			case SCRambl::Tokens::Type::String:
			case SCRambl::Tokens::Type::Character:
			case SCRambl::Tokens::Type::None:
				break;
			}
			type = type;
		}

		m_CodeIsOld = false;
	}

	m_IsParseActive = false;
}

void RunningProcessor::process()
{
	qDebug("Processing...");

	while (m_Engine.Run().GetState() != SCRambl::TaskSystem::Task<ProcessorTask::ID>::finished)
	{
		switch (m_Engine.GetCurrentTaskID()) {
		case ProcessorTask::Preprocessor: {
			auto& task = m_Engine.GetCurrentTask<SCRambl::Preprocessor::Task>();
			if (task.IsRunning()) {
				if (auto pos = task.Info().GetScriptPos())
				{
					//auto pc = std::floor(((float)pos.GetLine() / fNumLines) * 100.0);
					//std::cout << "Preprocessing..." << pc << "%" << "\r";
				}
			}
			break;
		}
		}
	}

	emit finished();
}

RunningProcessor::RunningProcessor(Processor& processor, SCRambl::Engine& eng) : m_Processor(processor), m_Engine(eng)
{ }
RunningProcessor::~RunningProcessor()
{ }