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
#include "TokensB.h"

namespace SCRambl
{
	namespace Parser
	{
		using Character = Preprocessor::Character;
		using Delimiter = Preprocessor::Delimiter;

		/*\ Parser::Token - Parser wrapper for script tokens \*/
		enum class ParsedType {
			Command, OLCommand,
		};
		/*\ Parser::Symbolic - Symbolic parser data \*/
		enum class SymbolTypes {
			Label, Command, Value
		};
		enum class CommandParam {
			Command = Tokens::Identifier::EXTRA,
		};

		using CommandInfo = Tokens::Identifier::Info < ParsedType, Command::Shared >;
		using OLCommandInfo = Tokens::Identifier::Info < ParsedType, Commands::Vector >;

		/*\ Parser::Symbols - Symbolic data for parsed scripts \*/
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

		/*\ Parser::Error - Errors that can happen while preprocessing \*/
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

		class Symbolic {
		public:
			inline virtual ~Symbolic()
			{ }
		};

		/*\
		 * Parser::Jump - Jumps from a code position to a label
		\*/
		class Jump : public Symbolic {
			Scripts::Tokens::Iterator m_TokenIt;
			Scripts::Label::Shared m_Dest;

		public:
			using Shared = std::shared_ptr < Jump > ;
			using Vector = std::vector < Jump > ;

			Jump(Scripts::Label::Shared dest, Scripts::Tokens::Iterator it) :
				m_Dest(dest), m_TokenIt(it)
			{ }
		};

		class Task;

		struct LabelRef {
			Scripts::Tokens::Iterator TokenIt;
			bool IsReference;
			size_t NumUses;

			LabelRef(Scripts::Tokens::Iterator it, bool isref) : TokenIt(it), IsReference(isref)
			{ }
		};
		
		/*\
		 * Parser::Parser - Now this is what we're here for
		\*/
		class Parser
		{
			using LabelMap = std::unordered_map < std::string, std::shared_ptr<Scripts::Label> > ;

		public:
			enum State {
				init, parsing, overloading, finished,
				bad_state, max_state = bad_state,
			};

			Parser(Task & task, Engine & engine, Script & script);

			bool IsFinished() const;
			bool IsRunning() const;
			void ParseOverloadedCommand();
			void Run();
			void Reset();

			size_t GetNumTokens() const;
			size_t GetCurrentToken() const;
			Scripts::Token GetToken() const;

		private:
			// Send an error event
			void SendError(Error);
			template<typename First, typename... Args> void SendError(Error, First&&, Args&&...);

			bool ParseCommandOverloads(const Commands::Vector & vec);
			void BeginCommandParsing();
			inline bool IsCommandParsing() const {
				return m_ParsingCommandArgs && m_CurrentCommand;
			}
			inline bool IsOverloading() const {
				return m_State == overloading;
			}
			inline bool AreCommandArgsParsed() const {
				return m_EndOfCommandArgs;
			}
			inline void NextCommandArg() {
				if (m_CurrentCommand) {
					if (!m_EndOfCommandArgs)
					{
						++m_CommandArgIt;
						++m_NumCommandArgs;
					}

					m_EndOfCommandArgs = m_CommandArgIt == m_CurrentCommand->EndArg();
				}
				else m_EndOfCommandArgs = true;
			}
			inline void NextCommandOverload() {
				++m_OverloadCommandsIt;
			}
			void FinishCommandParsing() {
				auto tok = m_CommandTokenIt.Get()->GetToken<Tokens::Command::Info<Command>>();
				m_CommandTokenIt.Get()->GetSymbol() = Tokens::CreateToken<Tokens::Command::Call<Command>>(*tok, m_NumCommandArgs);

				size_t cmdid = m_CommandVector.size();
				auto it = m_CommandMap.empty() ? m_CommandMap.end() : m_CommandMap.find(m_CurrentCommand->GetName());
				if (it != m_CommandMap.end()) {
					cmdid = it->second;
				}
				else {
					m_CommandMap.emplace(m_CurrentCommand->GetName(), m_CommandVector.size());
					m_CommandVector.emplace_back(m_CurrentCommand);
				}

				m_Script.GetDeclarations().emplace_back(Tokens::CreateToken <Tokens::Command::Decl<Command>>(cmdid, m_CurrentCommand));

				m_ParsingCommandArgs = false;
			}
			void MarkOverloadIncompatible() {
				m_OverloadCommandsIt = m_OverloadCommands.erase(m_OverloadCommandsIt);
			}
			void FailCommandOverload() {
				++m_NumOverloadFailures;
			}
			size_t GetNumberOfOverloadFailures() const {
				return m_NumOverloadFailures;
			}
			void AddLabel(Scripts::Label::Shared label, Scripts::Tokens::Iterator it) {
				m_LabelReferences.emplace(label, LabelRef(it, false));
			}
			void AddLabelRef(Scripts::Label::Shared label, Scripts::Tokens::Iterator it) {
				auto iter = m_LabelReferences.find(label);
				if (iter == m_LabelReferences.end()) {
					m_LabelReferences.emplace(label, LabelRef(it, true));
				}
				else {
					++iter->second.NumUses;
				}
			}

			inline Types::Type::Shared GetType(const std::string& name) {
				auto ptr = m_Types.GetType(name);
				return ptr ? ptr : m_Engine.GetTypes().GetType(name);
			}

			void Init();
			void Parse();

			State								m_State = init;
			Engine							&	m_Engine;
			Task							&	m_Task;
			Script							&	m_Script;
			Scripts::Tokens					&	m_Tokens;
			Scripts::Tokens::Iterator			m_TokenIt;
			Scripts::Tokens::Iterator			m_CommandTokenIt;
			Scripts::Tokens::Iterator::CVector	m_CommandTokens;			// positions of all parsed command tokens
			Scripts::Tokens::Iterator::CVector	m_LabelTokens;
			std::map<Scripts::Label::Shared, LabelRef>	m_LabelReferences;
			Scripts::Labels					&	m_Labels;
			Commands						&	m_Commands;
			Commands							m_ExtraCommands;
			Types::Types						m_Types;
			Command::Shared						m_CurrentCommand;
			Command::Arg::Iterator				m_CommandArgIt;
			size_t								m_NumCommandArgs;
			size_t								m_NumOverloadFailures;
			Commands::Vector					m_OverloadCommands;
			Commands::Vector::iterator			m_OverloadCommandsIt;
			Jump::Vector						m_Jumps;

			std::vector<std::shared_ptr<const Command>>		m_CommandVector;
			std::unordered_map<std::string, size_t>			m_CommandMap;

			// Status
			bool							m_OnNewLine;
			bool							m_ParsingCommandArgs;
			bool							m_EndOfCommandArgs;
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
		
		/*\ Parser::Task \*/
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
			inline Scripts::Token GetToken() const			{ return Parser::GetToken(); }

			bool IsRunning() const					{ return Parser::IsRunning(); }
			bool IsTaskFinished() final override	{ return Parser::IsFinished(); }

		protected:
			void RunTask() final override			{ Parser::Run(); }
			void ResetTask() final override			{ Parser::Reset(); }
		};
	}
}