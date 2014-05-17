/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Tasks.h"

namespace SCRambl
{
	class Builder : public Task
	{
	public:
		enum State {
			init, preprocess, parse, compile, link,
			finished
		};

	private:
		State		m_State;

		void Init();
		void Run();

	protected:
		void RunTask() override				{ Run(); }
		bool IsTaskFinished() override		{ return m_State == finished; }
		void ResetTask() override			{ Init(); }

	public:
	};
}