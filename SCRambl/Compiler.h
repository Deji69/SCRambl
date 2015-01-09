/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Tasks.h"
#include "Scripts.h"
#include "Parser.h"

namespace SCRambl
{
	namespace Compiler
	{
		class Task;

		class Compiler
		{
		public:
			enum State {
				init, compiling, finished,
				bad_state, max_state = bad_state,
			};

			Compiler(Task & task, Engine & engine, Script & script);

			inline bool IsFinished() const			{ return m_State == finished; }
			inline bool IsRunning()	const			{ return m_State == init || m_State == compiling; }
			void Init();
			void Run();
			void Reset();

		private:
			State				m_State;
			Task &				m_Task;
			Engine &			m_Engine;
			Script &			m_Script;
		};

		/*\
		 * Compiler::Event - Interesting stuff that the Preprocessor does
		\*/
		enum class Event
		{
			Begin, Finish,
			Warning,
			Error,
		};

		/*\
		 * Compiler::Task - The Task of being a Parser is a tough one (not really, programming is harder)
		\*/
		class Task : public TaskSystem::Task<Event>, private Compiler
		{
			friend Compiler;
			Engine				&	m_Engine;

			inline bool operator()(Event id)					{ return CallEventHandler(id); }
			template<typename... Args>
			inline bool operator()(Event id, Args&&... args)	{ return CallEventHandler(id, std::forward<Args>(args)...); }

		public:
			Task(Engine & engine, Script & script) :
				Compiler(*this, engine, script),
				m_Engine(engine)
			{ }

			//inline size_t GetProgressCurrent() const		{ return GetCurrentToken(); }
			//inline size_t GetProgressTotal() const			{ return GetNumTokens(); }
			//inline Script::Token GetToken() const			{ return Compiler::GetToken(); }

		protected:
			bool IsRunning() const					{ return Compiler::IsRunning(); }
			bool IsTaskFinished() final override	{ return Compiler::IsFinished(); }
			void RunTask() final override			{ Compiler::Run(); }
			void ResetTask() final override			{ Compiler::Reset(); }
		};
	}
}