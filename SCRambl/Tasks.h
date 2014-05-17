/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once

namespace SCRambl
{
	class Task
	{
		friend class Engine;

	public:
		enum State { running, error, finished };

	private:
		State		m_State;

	protected:
		virtual void ResetTask() = 0;
		virtual void RunTask() = 0;
		virtual bool IsTaskFinished() = 0;

		inline State & TaskState()			{ return m_State; }

	public:
		inline State GetState()	const		{ return m_State; }

		Task()
		{
			m_State = running;
		}
		virtual ~Task() { };

		const Task & Run()
		{
			do
			{
				switch (m_State)
				{
				case error:
					m_State = running;
				case running:
					try
					{
						RunTask();

						if (IsTaskFinished()) m_State = finished;
					}
					catch (...)
					{
						m_State = error;
					}
					break;
				case finished:
					ResetTask();
					if (!IsTaskFinished()) m_State = running;
					continue;
				}
			} while (false);

			return *this;
		};
	};

	/*class ParserState : Task
	{

	};

	class LinkerState : Task
	{

	};

	class CompilerState : Task
	{

	};

	class BuilderState : Task
	{
	public:
		enum State { init, preprocess, parse, link, compile, max_state };
		
	private:
		static const State state_branch[max_state];

		State m_State = init;

		PreprocessorState		*	m_pPreprocessor;

		SCRambl::Task & AdvanceState(Task::State & state)
		{
			m_State = state_branch[m_State];
			if (m_State == max_state)
			{
				state = Task::State::finished;
				m_State = init;
			}
			else state = Task::State::running;
			return *this;
		}

	public:
		inline State GetState() const { return m_State; }
	};

	class GlobalState : Task
	{
		BuilderState			*	m_Builder;
	};*/
}