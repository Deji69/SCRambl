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

namespace SCRambl
{
	class Parser
	{
	public:
		enum State {
			init, parsing, finished,
			bad_state,
			max_state = bad_state,
		};

		Parser(Engine & engine, Script & script);

		inline bool IsFinished()				{ return m_State == finished; }
		void Run();
		void Reset();

	private:
		State					m_State = init;
		Engine				&	m_Engine;
		Script				&	m_Script;
	};

	enum class ParserEvent
	{

	};

	class ParserTask : public TaskSystem::Task<ParserEvent>, public Parser
	{
	public:
		ParserTask(Engine & engine, Script & script) : Parser(engine, script)
		{ }

	protected:
		bool IsTaskFinished() final override	{ return Parser::IsFinished(); }
		void RunTask() final override			{ Parser::Run(); }
		void ResetTask() final override			{ Parser::Reset(); }
	};
}