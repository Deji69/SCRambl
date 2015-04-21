#ifndef UI_PARSER_H
#define UI_PARSER_H
#include "SCRambl.h"
#include "config.h"

class Processor;

class ProcessorTask {
public:
	enum ID { Preprocessor, Parser, Compiler, Linker };

private:
	ID				m_ID;

public:
	ProcessorTask(ID id) : m_ID(id)
	{ }
	inline operator ID&()						{ return m_ID; }
	inline operator const ID&() const			{ return m_ID; }
};
/*enum class ProcessorTask : int {
	Preprocessor, Parser, Compiler, Linker
};*/
enum class KeywordType {
	none, command, label, constant
};

class RunningProcessor : public QObject
{
	Q_OBJECT

public:
	RunningProcessor(Processor &, SCRambl::Engine &);
	~RunningProcessor();

public slots:
	void process();

signals:
	void finished();
	void error(QString err);

private:
	bool					m_Finished = false;
	SCRambl::Engine		&	m_Engine;
	Processor			&	m_Processor;
};

class Processor : public QObject, Configurable
{
	Q_OBJECT

	SCRambl::Engine					m_SCRamblEngine;
	SCRambl::Script				*	m_Script;
	RunningProcessor			*	m_Running;
	SCRambl::Script::TokenMap::Shared			m_TokenMap;
	std::shared_ptr<SCRambl::Preprocessor::Task>		m_Preprocessor;
	std::shared_ptr<SCRambl::Parser::Task>				m_Parser;

public:
	class Configuration : public ConfigBase
	{
		void applyDefaults() {
			if (QFile::exists("config\\Build.xml")) {
				QFileInfo fi("config\\Build.xml");
				BuildConfigFilePath = fi.absoluteFilePath();
			}
			onBuildConfigFileChange();
		}

	public:
		QString				BuildConfigFilePath;
		QString				BuildConfig;
		int					SelectedBuildConfig = 0;
		QStringList			AllBuildConfigs;

		Configuration(Configurable* configurable, bool isTemp = false) : ConfigBase("Processor", configurable, isTemp)
		{
			applyDefaults();

			// load, save
			ConfigBase::Load();
			ConfigBase::Save();
		}
		~Configuration() {
			ConfigBase::Save();
		}

		void selectBuildConfig(int n)
		{
			if (n < AllBuildConfigs.count()) {
				SelectedBuildConfig = n;
				BuildConfig = AllBuildConfigs[n];
			}
			else if (AllBuildConfigs.count() == 0) {
				SelectedBuildConfig = 0;
				BuildConfig = "";
			}
		}

		void onBuildConfigFileChange()
		{
			if (!BuildConfigFilePath.isEmpty()) {
				pugi::xml_document doc;
				doc.load_file(BuildConfigFilePath.toUtf8().constData());
				AllBuildConfigs.clear();
				auto node = doc.first_element_by_path("SCRambl/BuildConfig/Configuration");
				if (node) {
					do {
						if (!std::strcmp(node.name(), "Configuration")) {
							AllBuildConfigs.append(node.attribute("ID").as_string());
						}
					} while (node = node.next_sibling());
				}
			}

			selectBuildConfig(0);
		}

	private:
		void Load(const QSettings& settings) override
		{
			BuildConfigFilePath = settings.value("BuildConfigFilePath", BuildConfigFilePath).toString();
			BuildConfig = settings.value("BuildConfig", BuildConfig).toString();
		}

		void Save(QSettings& settings) override
		{
			settings.setValue("BuildConfigFilePath", BuildConfigFilePath);
			settings.setValue("BuildConfig", BuildConfig);
		}
	};

	Processor();
	Processor(const Processor&) = delete;
	~Processor();
	
	SCRambl::Script::TokenMap::Shared getTokenMap();
	KeywordType getKeywordType(const QString& kw);

	void setCode(const void * data)
	{
		if (!m_IsParseActive) {
			m_CodeIsOld = true;
			m_Script->SetCode(data);
		}
	}
	void parseCode()
	{
		if (!m_Running) {
			m_RunningThread = new QThread();
			m_Running = new RunningProcessor(*this, m_SCRamblEngine);
			m_Running->moveToThread(m_RunningThread);
			QCoreApplication::connect(m_Running, SIGNAL(RunningProcessor::error(QString)), this, SLOT(errorString(QString)));
			QCoreApplication::connect(m_RunningThread, SIGNAL(started()), m_Running, SLOT(process()));
			QCoreApplication::connect(m_Running, SIGNAL(finished()), this, SLOT(parseFinished()));
			QCoreApplication::connect(m_Running, SIGNAL(finished()), m_RunningThread, SLOT(quit()));
			QCoreApplication::connect(m_Running, SIGNAL(finished()), m_Running, SLOT(deleteLater()));
			QCoreApplication::connect(m_RunningThread, SIGNAL(finished()), m_RunningThread, SLOT(deleteLater()));
			m_RunningThread->start();
			m_IsParseActive = true;
		}
	}

	void onConfigUpdate() override;

public slots:
	void parseFinished();
	void errorString(QString);

private:
	bool						m_CodeIsOld = false;
	bool						m_IsParseActive = false;
	Configuration				m_Config;
	SCRambl::Project			m_Project;
	QThread					*	m_RunningThread = nullptr;
};

extern Processor& GetProcessor();

#endif