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
		//preparser,
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
		try
		{
			script.LoadFile(path);
		}
		catch (...)
		{
			std::cerr << "FATAL ERROR: failed to open file \'" << path << "\'\n";
		}

		std::cout << "Loaded. " << script.GetCode().NumLines() << " lines, " << script.GetCode().NumSymbols() << " symbols.\n";

		try
		{
			// Add the preprocessor task to preprocess the script - give it our 'preprocessor' ID so we can identify it later
			auto task = engine.AddTask<SCRambl::Preprocessor::Task>(preprocessor, std::ref(script));
			
			using SCRambl::Preprocessor::Event;
			/*auto Preprocessor_Warning = [](SCRambl::Preprocessor::Warning id, std::string msg){
				std::cout << "warning ("<< id <<"): "<< msg;
				return true;
			};*/

			task->AddEventHandler<Event::Begin>([](){
				std::cout << "Preprocessing... \n";
				return true;
			});
			//task->AddEventHandler<Event::Warning>(Preprocessor_Warning);

			// Add event handler for preprocessor errors
			task->AddEventHandler<Event::Error>([script,task](SCRambl::Basic::Error id, std::vector<std::string>& params){
				using SCRambl::Preprocessor::Error;

				// get some much needed info, display the file, line number and error ID
				auto & pos = task->Info().GetScriptPos();
				auto script_file = pos.GetLine().GetFile();
				std::cerr << script_file->GetPath() << "(" << pos.GetLine() << "," << pos.GetColumn() << ")> error (" << id.Get<SCRambl::Preprocessor::Error>() << "): ";

				// 
				switch (id.Get<SCRambl::Preprocessor::Error>()) {
				default:
				{
					bool b = false;
					for (auto p : params) {
						if (!p.empty())
						{
							if (b) std::cerr << ", ";
							else b = true;
							std::cerr << p;
						}
					}
					break;
				}
				case Error::invalid_directive: std::cerr << "invalid directive '" << params[0] << "'";
					break;
				case Error::unterminated_block_comment: std::cerr << "unterminated block comment";
					break;
				case Error::unterminated_string_literal: std::cerr << "unterminated string literal";
					break;
				}

				std::cerr << "\n";
				return true;
			});

			task->AddEventHandler<Event::FoundToken>([](SCRambl::Script::Range range){
				std::cerr << ">>>" << range.Formatter(range) << "\n";
				return true;
			});

			// Add the parser task to parse the code symbols to tokens
			engine.AddTask<SCRambl::ParserTask>(parser, script);

			//
			bool bRunning = true;
			bool bPreprocessorStarted = false;

			// main loop
			using SCRambl::TaskSystem::Task;
			while (engine.Run().GetState() != finished)
			{
			}
		}
		catch (const std::exception & ex)
		{
			std::cout << "ERROR std::exception: '" << ex.what() << "'\n";
			return 1;
		}
		catch (...)
		{
			std::cout << "ERROR: failed to compile file '" << "'\n";
			return 1;
		}
	}

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