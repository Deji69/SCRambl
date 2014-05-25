/****************************************************/
// SCRambl - main.cpp
// Main console window
/****************************************************/

#include "stdafx.h"
#include "SCRambl.h"
#include "main.h"

void ProcessCommand(const std::string & cmd);

int main(int argc, char* argv[])
{
	std::string cmd;
	CCLP CmdParser({ argv + 1, &argv[argc] });

	CmdParser.AddFlag("format", 'f');
	CmdParser.AddFlag("output", 'o');
	CmdParser.AddFlag("help", 'h');
	CmdParser.Parse();

	std::cout << "SCRambl Advanced SCR Compiler/Assembler\n";

	if (CmdParser.IsFlagSet("help") || CmdParser.IsFlagSet("?") || !CmdParser.GetOpts().size())
	{
		std::list<std::string> helps(CmdParser.GetFlagOpts("help"));
		auto qs = CmdParser.GetFlagOpts("?");
		helps.insert(helps.end(), qs.begin(), qs.end());

		if (!helps.size()) helps = CmdParser.GetFlagOpts("?");
		if (!helps.size())
		{
			std::cout << "Syntax: SCRambl <input_filename> [-f=<format>] [-o=<output_filename>]\n"
				<< "Use the -h flag for help on any other flag e.g. \"-hf\" or \"-h format\" for help on the -f flag\n";
			return 0;
		}
		else
		{
			for (auto help : helps)
			{
				if (help == "f")
				{
					std::cout << "Specifies the output format. Use one of the following format parameters:\n"
						<< "scc - Single Compiled Script\n"
						<< "scm - Script Multifile\n"
						<< "Syntax: -f <format>";
				}
			}
		}
	}

	// Initiate SCRambl engine
	SCRambl::Engine engine;

	enum Task
	{
		preparser,
		preprocessor,
		parser,
		compiler,
		linker,
		finished
	};

	// Initialise script
	SCRambl::Script script;

	// Load a script from each file and feed it into the SCRambl engine
	auto files = CmdParser.GetOpts();
	for (auto path : files)
	{
		engine.AddTask<Task, SCRambl::PreparserTask>(preparser, script, path);
	}
		try
		{
			// Add a preparser task to read the script into a simpler format
			//engine.AddTask<Task, SCRambl::PreparserTask, SCRambl::Script>(preparser, script);

			// Add a preprocessor task to preprocess the script - give it our 'preprocessor' ID so we can identify it later
			//engine.AddTask<Task, SCRambl::PreprocessorTask, SCRambl::Script>(preprocessor, script);

			//
			bool bRunning = true;
			bool bPreprocessorStarted = false;
			do
			{
				using SCRambl::Task;
				switch (engine.Run().GetState())
				{
				case Task::running:
					// output errors, warnings and status
					switch (engine.GetCurrentTaskID())
					{
					case preprocessor:
						switch (engine.GetCurrentTask<SCRambl::PreprocessorTask>().GetState())
						{
						case Task::running:
							if (!bPreprocessorStarted)
							{
								std::cout << "Preprocessing..." << "\n";
								bPreprocessorStarted = true;
							}
							break;
						case Task::finished:
							break;
						case Task::error:
							std::cout << "ERROR (Preprocessor) : " << "\n";
							break;
						}
						break;
					}
					/*switch (engine.GetState())
					{
					}*/
					break;
				case Task::finished:
					//
					std::cout << "Finished." << "\n";
					bRunning = false;
					break;
				case Task::error:
					std::cout << "FATAL ERROR: " << "\n";
					bRunning = false;
					break;
				}
			} while (bRunning);
		}
		catch (...)
		{
			std::cout << "ERROR: failed to compile file '" << "'\n";
			return 1;
		}
	//}

	//SCRambl::
	return 0;
}

void ProcessCommand(const std::string & cmd)
{
	if (cmd == "hash")
	{
		std::string str;
		std::cin >> str;
		std::cout << "I told you to go away" << std::endl;
	}
}