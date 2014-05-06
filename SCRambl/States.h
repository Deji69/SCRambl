/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once

namespace SCRambl
{
	class RunningState
	{
	public:
		enum eState { running, error, finished };

	private:
		eState m_State = running;

	public:
		eState & State() { return m_State; }
		eState State() const { return m_State; }
	};

	class Task
	{
	public:
		enum State { running, error, finished };
		
	private:
		State m_State;

	protected:
		//virtual void RunTask() = 0;
		virtual Task & RunTask() = 0;
		virtual Task & AdvanceState(State & state) = 0;

		State	&	TaskState();

	public:
		inline State GetState()	const		{ return m_State; }

		Task()
		{
			m_State = running;
		}

		const Task & Run()
		{
			switch (m_State)
			{
			case error:
				m_State = running;
			case running:
				try
				{
					m_State = RunTask().Run().GetState();
				}
				catch (...)
				{
					m_State = error;
				}
				break;
			case finished:
				m_State = AdvanceState(m_State).GetState();
				break;
			}
			return *this;
		};
	};

	class PreprocessorState : protected Task
	{
	public:
		enum State {
			before_directive, during_directive, after_directive,
			before_comment, during_comment, after_comment,
			max_state
		};
		enum Operation {
			directive,
			comment,
			max_operation
		};
	};

	class ParserState : protected Task
	{

	};

	class LinkerState : protected Task
	{

	};

	class CompilerState : protected Task
	{

	};

	class BuilderState : protected Task
	{
	public:
		enum State { init, preprocess, parse, link, compile, max_state };
		
	private:
		static const State state_branch[max_state];

		State m_State = init;

		PreprocessorState		*	m_pPreprocessor;

		void AdvanceState(RunningState & state)
		{
			m_State = state_branch[m_State];
			if (m_State == max_state)
			{
				state.State() = RunningState::finished;
				m_State = init;
			}
			else state.State() = RunningState::running;
		}

	public:
		inline State GetState() const { return m_State; }
	};

	class GlobalState
	{
		BuilderState			*	m_Builder;
	};
}