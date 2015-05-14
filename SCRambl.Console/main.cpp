/****************************************************/
// SCRambl - main.cpp
// Main console window
/****************************************************/

#include "stdafx.h"
#include "SCRambl.h"
#include "main.h"
#include "SCRambl\XML.h"

void ProcessCommand(const std::string & cmd);

int main(int argc, char* argv[])
{
	std::string cmd;
	CCLP CmdParser({ argv + 1, &argv[argc] });

	CmdParser.AddFlag("build", 'b');
	CmdParser.AddFlag("format", 'f');
	CmdParser.AddFlag("help", 'h');
	CmdParser.AddFlag("load", 'l');
	CmdParser.AddFlag("output", 'o');
	CmdParser.AddFlag("project", 'p');
	CmdParser.Parse();

	SCRambl::XML xml("build\\build.xml");
	auto node = xml.GetNode("BuildConfig");
	auto conf = node.GetNode("Build");
	auto attr = conf.GetAttribute("Name");
	auto val = attr.GetValue();
	auto name = val.AsString();
	
	return 0;

	std::cout << "SCRambl Advanced SCR Compiler/Assembler\n";
	bool error_status = false;

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
					std::cout << "Specifies the output format. Use one of the following format parameters:\n"
						<< "scc - Single Compiled Script\n"
						<< "scm - Script Multifile\n"
						<< "Syntax: -f <format>";
				else if (help == "p")
					std::cout << "Enables creation of a project file. Use this with -l to set file path.\n"
						<< "Syntax: -p <projectname (optional)>";
				else if (help == "l")
					std::cout << "Sets the project file path. Without -p this loads a project, otherwise it sets the save path.\n"
						<< "Syntax: -l <filename>";
			}
		}
	}

	auto& project_opts = CmdParser.GetFlagOpts("project");
	auto& load_opts = CmdParser.GetFlagOpts("load");
	bool build_project = !project_opts.empty();
	bool load_project = !load_opts.empty();
	std::string project_name;

	// Initiate SCRambl engine
	SCRambl::Engine engine;
	using SCRambl::EngineEvent;

	engine.AddEventHandler<EngineEvent::ConfigurationError>([](SCRambl::Basic::Error id, std::vector<std::string>& params){
		
	});

	// Load configuration
	std::cout << "Loading configuration...\n";
	std::cout << "Loading build configuration...\n";
	if (!engine.LoadBuildFile("build\\build.xml", CmdParser.GetFlagOpts("build").front())) {
		std::cerr << "Failed to load build configuration!";
	}

	SCRambl::Project project;

	// Build project file
	build_project = false;
	if (!load_project && build_project) {
		project.SetName(project_opts.front());
		project.SetConfig(engine.GetBuildConfig());
		
		if (load_project) project.SaveFile(load_opts.front().c_str());
		else project.SaveFile();

		auto files = CmdParser.GetOpts();
		for (auto path : files)
		{
			project.AddSource(path);
		}
	}
	else if (load_project) {
		try {
			project.LoadFile(load_opts.front().c_str());
		} 
		catch (...) {
			std::cerr << "ERROR: Failed to load project file \"" << load_opts.front().c_str() << "\"" << std::endl;
			error_status = true;
		}
	}

	if (error_status) {
		for (int i = 0; i < 6; ++i) {
			Sleep(1000);
			std::cout << ".";
		}
	}

	enum Task
	{
		preprocessor,
		parser,
		compiler,
		linker,
		finished
	};

	// Initialise script
	SCRambl::Script script;
	for (auto& path : CmdParser.GetOpts()) {
		script.OpenFile(path);
	}

	std::cout << "Loaded. " << script.GetCode().NumLines() << " lines, " << script.GetCode().NumSymbols() << " symbols.\n";

	/**************** Preprocessor Stuff ****************/
	// Add the preprocessor task to preprocess the script - give it our 'preprocessor' ID so we can identify it later
	auto preprocessor_task = engine.AddTask<SCRambl::Preprocessor::Task>(preprocessor, std::ref(script));
	auto parser_task = engine.AddTask<SCRambl::Parser::Task>(parser, std::ref(script));
	
	using TaskSys = SCRambl::TaskSystem::Task<Task>;
	float fNumLines = (float)script.GetCode().NumLines();
	while (engine.Run().GetState() != TaskSys::finished)
	{
		switch (engine.GetCurrentTaskID()) {
		case preprocessor: {
			auto& task = engine.GetCurrentTask<SCRambl::Preprocessor::Task>();
			if (task.IsRunning()) {
				if (auto pos = task.Info().GetScriptPos())
				{
					auto pc = std::floor(((float)pos.GetLine() / fNumLines) * 100.0);
					std::cout << "Preprocessing..." << pc << "%" << "\r";
				}
			}
			break;
		}
		case parser: {
			auto& task = engine.GetCurrentTask<SCRambl::Parser::Task>();
			if (task.IsRunning()) {
				auto pc = std::floor(((float)task.GetProgressCurrent() / (float)task.GetProgressTotal()) * 100.0);
				std::cout << "Parsing..." << pc << "%" << "\r";
			}
			break;
		}
		case compiler: {
			auto& task = engine.GetCurrentTask<SCRambl::Compiler::Task>();
			if (task.IsRunning()) {
				auto pc = std::floor(((float)task.GetProgressCurrent() / (float)task.GetProgressTotal()) * 100.0);
				std::cout << "Compiling..." << pc << "%" << "\r";
			}
			break;
		}
		}
	}
	return 0;
#if 0
	// Load a script from each file and feed it into the SCRambl engine
	auto files = CmdParser.GetOpts();
	for (auto path : files)
	{
		try
		{
			engine.LoadFile(path, script);
		}
		catch (...)
		{
			std::cerr << "FATAL ERROR: failed to open file \'" << path << "\'\n";
		}

		std::cout << "Loaded. " << script.GetCode().NumLines() << " lines, " << script.GetCode().NumSymbols() << " symbols.\n";

		try
		{
			/**************** Preprocessor Stuff ****************/
			// Add the preprocessor task to preprocess the script - give it our 'preprocessor' ID so we can identify it later
			auto preprocessor_task = engine.AddTask<SCRambl::Preprocessor::Task>(preprocessor, std::ref(script));

			preprocessor_task->AddEventHandler<SCRambl::Preprocessor::Event::Begin>([](){
				std::cout << "\nPreprocessing started.\n";
				return true;
			});
			preprocessor_task->AddEventHandler<SCRambl::Preprocessor::Event::Finish>([](){
				std::cout << "\nPreprocessing finished.\n";
				return true;
			});

			bool print_nl;

			// Add event handler for preprocessor errors
			preprocessor_task->AddEventHandler<SCRambl::Preprocessor::Event::Error>([&print_nl, &script, &preprocessor_task](SCRambl::Basic::Error id, std::vector<std::string>& params){
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
			preprocessor_task->AddEventHandler<SCRambl::Preprocessor::Event::AddedToken>([](SCRambl::Script::Range& range){
				return true;
			});
			preprocessor_task->AddEventHandler<SCRambl::Preprocessor::Event::FoundToken>([&print_nl](SCRambl::Script::Range range){
				return true;
			});

			/**************** Parser Stuff ****************/
			// Add the parser task to parse the code symbols to tokens
			auto parser_task = engine.AddTask<SCRambl::Parser::Task>(parser, script);
			parser_task->AddEventHandler<SCRambl::Parser::Event::Begin>([](){
				std::cout << "\nParsing started.\n";
				return true;
			});
			parser_task->AddEventHandler<SCRambl::Parser::Event::Finish>([](){
				std::cout << "\nParsing finished.\n";
				return true;
			});

			// Add event handler for parser errors
			parser_task->AddEventHandler<SCRambl::Parser::Event::Error>([&print_nl, &script, &parser_task](SCRambl::Basic::Error id, std::vector<std::string>& params){
				using SCRambl::Parser::Error;

				// get some much needed info, display the file, line number and error ID
				auto tok = parser_task->GetToken();
				auto & pos = tok.GetPosition();
				auto script_file = pos.GetLine().GetFile();
				auto error_id = id.Get<SCRambl::Parser::Error>();
				bool fatal = error_id >= Error::fatal_begin && error_id <= Error::fatal_end;

				// "  %s(%d,%d)> {fatal} error(%d) : "
				// e.g. "  file.sc(6,9)> fatal error(4001) : "
				std::cerr << "  " << script_file->GetPath() << "(" << pos.GetLine() << "," << pos.GetColumn()
						  << ")> " << (fatal ? "fatal error" : "error") << "(" << error_id << ") : ";

				switch (error_id)
				{
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

					// errors
				case Error::label_on_line: std::cerr << "unexpected label on current line " << params[0];
					break;
				}

				std::cerr << "\n";
				return true;
			});

			/**************** Compiler Stuff ****************/
			// Add the parser task to parse the code symbols to tokens
			auto compiler_task = engine.AddTask<SCRambl::Compiler::Task>(compiler, script);
			compiler_task->AddEventHandler<SCRambl::Compiler::Event::Begin>([](){
				std::cout << "\nCompilation started.\n";
				return true;
			});
			compiler_task->AddEventHandler<SCRambl::Compiler::Event::Finish>([](){
				std::cout << "\nCompilation finished.\n";
				return true;
			});

			/**************** Running... ****************/
			// main loop
			using TaskSys = SCRambl::TaskSystem::Task<Task>;
			float fNumLines = (float)script.GetCode().NumLines();
			while (engine.Run().GetState() != TaskSys::finished)
			{
				switch (engine.GetCurrentTaskID()) {
				case preprocessor: {
					auto& task = engine.GetCurrentTask<SCRambl::Preprocessor::Task>();
					if (task.IsRunning()) {
						if (auto pos = task.Info().GetScriptPos())
						{
							auto pc = std::floor(((float)pos.GetLine() / fNumLines) * 100.0);
							std::cout << "Preprocessing..." << pc << "%" << "\r";
						}
					}
					break;
				}
				case parser: {
					auto& task = engine.GetCurrentTask<SCRambl::Parser::Task>();
					if (task.IsRunning()) {
						auto pc = std::floor(((float)task.GetProgressCurrent() / (float)task.GetProgressTotal()) * 100.0);
						std::cout << "Parsing..." << pc << "%" << "\r";
					}
					break;
				}
				case compiler: {
					auto& task = engine.GetCurrentTask<SCRambl::Compiler::Task>();
					if (task.IsRunning()) {
						auto pc = std::floor(((float)task.GetProgressCurrent() / (float)task.GetProgressTotal()) * 100.0);
						std::cout << "Compiling..." << pc << "%" << "\r";
					}
					break;
				}
				}
			}

			std::cout << "                                                                         \r";
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

	return 0;
#endif
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