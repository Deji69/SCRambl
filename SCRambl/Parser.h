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

namespace SCRambl
{
	namespace Parser
	{
		using Token = Preprocessor::Token;

		/*\
		 * Parser::Symbols - Symbolic data for parsed scripts
		\*/
		class Symbols
		{
		public:
			class Object
			{
			};
		};

		/*\
		 * Parser::Error - Errors that can happen while preprocessing
		\*/
		class Error
		{
		public:
			enum ID {
				// involuntary errors (errors that should be impossible)
				//internal_invalid_unary_operator = 500,

				// normal errors
				//invalid_directive = 1000,

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

		class Task;
		
		/*\
		 * Parser::Parser - Now this is what we're here for
		\*/
		class Parser
		{
		public:
			enum State {
				init, parsing, finished,
				bad_state,
				max_state = bad_state,
			};

			Parser(Task & task, Engine & engine, Script & script);

			inline bool IsFinished() const			{ return m_State == finished; }
			inline bool IsRunning()	const			{ return m_State == init || m_State == parsing; }
			void Run();
			void Reset();

			inline size_t GetNumTokens() const {
				return m_Tokens.Size();
			}
			inline size_t GetCurrentToken() const {
				return m_TokenIt.GetIndex();
			}

		private:
			State						m_State = init;
			Engine					&	m_Engine;
			Task					&	m_Task;
			Script					&	m_Script;
			Script::Tokens			&	m_Tokens;
			Script::Tokens::Iterator	m_TokenIt;

			// Send an error event
			void SendError(Error);
			template<typename First, typename... Args> void SendError(Error, First&&, Args&&...);

			void Init();
			void Parse();
		};
		
		/*\
		 * Parser::Event - Interesting stuff that the Preprocessor does
		\*/
		enum class Event
		{
			Begin,
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

			inline bool operator()(Event id)			{ return CallEventHandler(id); }

			template<typename... Args>
			inline bool operator()(Event id, Args&&... args)	{ return CallEventHandler(id, std::forward<Args>(args)...); }

		public:
			Task(Engine & engine, Script & script) : Parser(*this, engine, script)
			{ }

			inline size_t GetProgressCurrent() const	{ return GetCurrentToken(); }
			inline size_t GetProgressTotal() const		{ return GetNumTokens(); }

		protected:
			bool IsRunning() const					{ return Parser::IsRunning(); }
			bool IsTaskFinished() final override	{ return Parser::IsFinished(); }
			void RunTask() final override			{ Parser::Run(); }
			void ResetTask() final override			{ Parser::Reset(); }
		};
	}
}