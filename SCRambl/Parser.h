/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include "utils.h"
#include "Engine.h"
#include "Scripts.h"
#include "Preprocessor.h"
#include "Labels.h"

namespace SCRambl
{
	namespace Parser
	{
		using Character = Preprocessor::Character;

		/*\
		 * Parser::Token - Parser wrapper for script tokens
		\*/
		class Token : public Preprocessor::Token
		{
			//Script::Token			m_Token;

		public:
			enum ParsedType {
				Command, OLCommand,
			};

			using CommandInfo = Identifier::Info < ParsedType, Command::Shared >;
			using OLCommandInfo = Identifier::Info < ParsedType, Commands::Vector >;

			//Token() = default;

			//inline const Script::Position & GetScriptPos() const			{ return m_Token.GetPosition(); }
		};

		/*\
		 * Parser::Symbols - Symbolic data for parsed scripts
		\*/
		class Symbols
		{
		public:
			enum class Structure {
				Header, Data
			};
			enum class DataStructure {

			};

			class Data
			{
				Types::ValueToken			m_ValToken;
			};

			class FileFunctions
			{
			};

			template<typename T = FileFunctions, typename... TArgs>
			void WriteFile(TArgs&&... args) {
				T();
			}
		};

		/*\
		 * Parser::Error - Errors that can happen while preprocessing
		\*/
		class Error
		{
		public:
			enum ID {
				// involuntary errors (errors that should be impossible)
				invalid_character		= 500,

				// normal errors
				invalid_identifier		= 1000,
				label_on_line			= 1001,
				unsupported_value_type	= 1002,

				// fatal errors
				fatal_begin = 4000,
				//include_failed = 4000,
				fatal_end,
			};

			Error(ID id) : m_ID(id)
			{ }
			inline operator ID() const			{ return m_ID; }

		private:
			ID			m_ID;
		};


		/*\
		 * Parser::Symbolic - Symbolic parser data
		\*/
		enum class SymbolTypes {
			Label, Command, Value
		};
		class Symbolic
		{
		public:
			inline virtual ~Symbolic()
			{ }
		};

		/*\
		 * Parser::Jump - Jumps from a code position to a label
		\*/
		class Jump : public Symbolic
		{
			Script::Tokens::Iterator	m_TokenIt;
			Script::Label::Shared		m_Dest;

		public:
			using Shared = std::shared_ptr < Jump > ;
			using Vector = std::vector < Jump > ;

			Jump(Script::Label::Shared dest, Script::Tokens::Iterator it) :
				m_Dest(dest), m_TokenIt(it)
			{ }
		};

		class Task;
		
		/*\
		 * Parser::Parser - Now this is what we're here for
		\*/
		class Parser
		{
			using LabelMap = std::unordered_map < std::string, std::shared_ptr<Script::Label> > ;

		public:
			enum State {
				init, parsing, overloading, finished,
				bad_state, max_state = bad_state,
			};

			Parser(Task & task, Engine & engine, Script & script);

			inline bool IsFinished() const			{ return m_State == finished; }
			inline bool IsRunning()	const			{ return m_State == init || m_State == parsing; }
			void ParseOverloadedCommand();
			void Run();
			void Reset();

			inline size_t GetNumTokens() const {
				return m_Tokens.Size();
			}
			inline size_t GetCurrentToken() const {
				return m_TokenIt.GetIndex();
			}
			inline Script::Token GetToken() const {
				return m_TokenIt.Get();
			}

		private:
			State							m_State = init;
			Engine						&	m_Engine;
			Task						&	m_Task;
			Script						&	m_Script;
			Script::Tokens				&	m_Tokens;
			Script::Tokens::Iterator		m_TokenIt;
			Script::Tokens::Iterator		m_CommandTokenIt;
			Script::Labels				&	m_Labels;
			Commands					&	m_Commands;
			Command::Shared					m_CurrentCommand;
			Command::Arg::Iterator			m_CommandArgIt;
			//SCR::Command::Arg
			Commands::Vector				m_OverloadCommands;
			Commands::Vector::iterator		m_OverloadCommandsIt;
			Jump::Vector					m_Jumps;

			// Status
			bool							m_OnNewLine;
			bool							m_ParsingCommandArgs;

			// Send an error event
			void SendError(Error);
			template<typename First, typename... Args> void SendError(Error, First&&, Args&&...);

			inline bool ParseCommandOverloads(const Commands::Vector & vec) {
				if (vec.size() == 1) {
					m_CurrentCommand = vec[0];
				}
				else {
					if (!m_OverloadCommands.empty()) {
						m_OverloadCommands = vec;
						m_OverloadCommandsIt = m_OverloadCommands.begin();
						m_CurrentCommand = *m_OverloadCommandsIt;
						m_State = overloading;
					}
					else {
						m_CurrentCommand = nullptr;
						return false;
					}
				}
				return true;
			}
			inline void BeginCommandParsing() {
				m_CommandTokenIt = m_TokenIt;
				if (m_State == overloading) {
					m_CurrentCommand = *m_OverloadCommandsIt;
				}
				if (m_CurrentCommand->GetNumArgs()) {
					m_CommandArgIt = m_CurrentCommand->BeginArg();
					m_ParsingCommandArgs = true;
				}
			}
			inline bool IsCommandParsing() {
				return m_ParsingCommandArgs;
			}
			inline void NextCommandArg() {
				++m_CommandArgIt;
				m_ParsingCommandArgs = m_CommandArgIt != m_CurrentCommand->EndArg();
			}

			void Init();
			void Parse();
		};
		
		/*\
		 * Parser::Event - Interesting stuff that the Preprocessor does
		\*/
		enum class Event
		{
			Begin, Finish,
			Warning,
			Error,
			FoundToken,
		};
		
		/*\
		 * Parser::Task - The Task of being a Parser is a tough one (not really, programming is harder)
		\*/
		class Task : public TaskSystem::Task<Event>, private Parser
		{
			friend Parser;
			Engine				&	m_Engine;

			inline bool operator()(Event id)					{ return CallEventHandler(id); }
			template<typename... Args>
			inline bool operator()(Event id, Args&&... args)	{ return CallEventHandler(id, std::forward<Args>(args)...); }

		public:
			Task(Engine & engine, Script & script) :
				Parser(*this, engine, script),
				m_Engine(engine)
			{ }

			inline size_t GetProgressCurrent() const		{ return GetCurrentToken(); }
			inline size_t GetProgressTotal() const			{ return GetNumTokens(); }
			inline Script::Token GetToken() const			{ return Parser::GetToken(); }

		protected:
			bool IsRunning() const					{ return Parser::IsRunning(); }
			bool IsTaskFinished() final override	{ return Parser::IsFinished(); }
			void RunTask() final override			{ Parser::Run(); }
			void ResetTask() final override			{ Parser::Reset(); }
		};
	}
}