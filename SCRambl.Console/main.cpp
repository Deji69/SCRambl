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
	using SCRambl::EngineEvent;

	engine.AddEventHandler<EngineEvent::ConfigurationError>([](SCRambl::Basic::Error id, std::vector<std::string>& params){
		
	});

	// Load configuration
	std::cout << "Loading configuration...\n";
	
	std::cout << "Loading constants from \"config\\gtasa\\constants.xml\"...\n";
	engine.LoadConfigFile("config\\gtasa\\constants.xml");
	
	std::cout << "Loading types from \"config\\gtasa\\types.xml\"...\n";
	engine.LoadConfigFile("config\\gtasa\\types.xml");

	std::cout << "Loading commands from \"config\\gtasa\\commands.xml\"...\n";
	engine.LoadConfigFile("config\\gtasa\\commands.xml");

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
			using SCRambl::Preprocessor::Event;

			// Add the preprocessor task to preprocess the script - give it our 'preprocessor' ID so we can identify it later
			auto preprocessor_task = engine.AddTask<SCRambl::Preprocessor::Task>(preprocessor, std::ref(script));
			
			/*auto Preprocessor_Warning = [](SCRambl::Preprocessor::Warning id, std::string msg){
				std::cout << "warning ("<< id <<"): "<< msg;
				return true;
			};*/

			preprocessor_task->AddEventHandler<Event::Begin>([](){
				std::cout << "\nPreprocessing started.\n";
				return true;
			});

			bool print_nl;

			// Add event handler for preprocessor errors
			preprocessor_task->AddEventHandler<Event::Error>([&print_nl, &script, &preprocessor_task](SCRambl::Basic::Error id, std::vector<std::string>& params){
				using SCRambl::Preprocessor::Error;

				if (print_nl) std::cerr << "\n";

				// get some much needed info, display the file, line number and error ID
				auto & pos = preprocessor_task->Info().GetScriptPos();
				auto script_file = pos.GetLine().GetFile();
				auto error_id = id.Get<SCRambl::Preprocessor::Error>();
				bool fatal = error_id >= Error::fatal_begin && error_id <= Error::fatal_end;

				// "  %s(%d,%d)> {fatal} error(%d) : "
				// e.g. "  file.sc(6,9)> fatal error(4001) : "
				std::cerr	<< "  " << script_file->GetPath() << "(" << pos.GetLine() << "," << pos.GetColumn()
							<< ")> " << (fatal ? "fatal error" : "error") << "(" << error_id << ") : ";

				// 
				switch (id.Get<SCRambl::Preprocessor::Error>()) {
				default:
					// unknown error? print all available params
					bool b;
					b = false;
					for (auto p : params) {
						if (!p.empty())
						{
							if (b) std::cerr << ", ";
							else b = true;
							std::cerr << p;
						}
					}
					break;
					// fatal errors
				case Error::include_failed: std::cerr << "failed to include file '" << params[0] << "'";
					break;

					// errors
				case Error::invalid_directive: std::cerr << "invalid directive '" << params[0] << "'";
					break;
				case Error::unterminated_block_comment: std::cerr << "unterminated block comment";
					break;
				case Error::unterminated_string_literal: std::cerr << "unterminated string literal";
					break;
				case Error::expr_expected_operator: std::cerr << "expected an operator (found '" << params[0] << "')";
					break;
				case Error::expr_unexpected_float: std::cerr << "float in integer constant expression '" << params[0] << "'";
					break;
				case Error::expr_unmatched_closing_parenthesis: std::cerr << "unmatched closing parenthesis '" << params[0] << "'";
					break;
				case Error::expr_invalid_operator: std::cerr << "forbidden operator '" << params[0] << "' in constant expression";
					break;
				case Error::expected_expression: std::cerr << "expected an expression";
					break;
				case Error::expected_identifier: std::cerr << "expected an identifier (found '" << params[0] << "')";
					break;
				case Error::invalid_unary_operator: std::cerr << "invalid unary operator '" << params[0] << "'";
					break;
				case Error::invalid_unary_operator_use: std::cerr << "invalid use of unary operator '" << params[0] << "'";
					break;
				case Error::dir_expected_file_name: std::cerr << "'" << params[0] << "' expected a file name";
					break;
				}

				std::cerr << "\n";
				return true;
			});

			preprocessor_task->AddEventHandler<Event::FoundToken>([&print_nl](SCRambl::Script::Range range){
				//std::cerr << ">>>" << range.Formatter(range) << "\n";
				print_nl = true;
				return true;
			});

			// Add the parser task to parse the code symbols to tokens
			engine.AddTask<SCRambl::ParserTask>(parser, script);

			//
			bool bRunning = true;
			bool bPreprocessorStarted = false;

			// main loop
			using SCRambl::TaskSystem::Task;
			float fNumLines = (float)script.GetCode().NumLines();
			while (engine.Run().GetState() != finished)
			{
				switch (engine.GetCurrentTaskID()) {
				case preprocessor: {
					auto& task = engine.GetCurrentTask<SCRambl::Preprocessor::Task>();
					std::cout << "Preprocessing...";
					if (auto pos = task.Info().GetScriptPos())
					{
						auto pc = std::floor(((float)pos.GetLine() / fNumLines) * 100.0);
						std::cout << pc << "%" << "\r";
					}
					break;
				}
				case parser: {
					auto& task = engine.GetCurrentTask<SCRambl::Parser>();
					std::cout << "Parsing...\r";
					//std::cout << std::floor(((float)task.->Info().GetScriptPos().GetLine() / (float)script.GetCode().NumLines()) * 100.0) << "%" << "\r";
					break;
				}
				}
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