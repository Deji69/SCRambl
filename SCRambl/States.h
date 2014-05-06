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
		virtual void RunTask() = 0;
		virtual void AdvanceState(State & state) = 0;

		State	&	TaskState();
	public:
		Task()
		{
			m_State = running;
		}
		const State & Run()
		{
			// the state has started as...
			switch (m_State)
			{
			case error:
				m_State = running;
			case running:
				RunTask();
				break;
			case finished:
				AdvanceState(m_State);
				break;
			}
			// ... ends up as
			return m_State;
		};
	};

	class PreprocessorState : public Task
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

	class ParserState : public Task
	{

	};

	class LinkerState : public Task
	{

	};

	class CompilerState : public Task
	{

	};

	class BuilderState : public Task
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
		}

	};

	class GlobalState
	{
		BuilderState			*	m_Builder;
	};
}