/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include <fstream>
#include "Configuration.h"
#include "Tasks.h"
#include "Scripts.h"

namespace SCRambl
{
	class PreparserConfig
	{

	};

	class Preparser
	{
	public:
		enum State {
			init,
			reading,
			parsing,
			end_of_line,
			continue_next_line,
			finished,
			bad_state,
		};

	private:
		State					m_State;
		Engine				&	m_Engine;
		Script				&	m_Script;
		long					m_NumLines;
		CodeLine				m_ParsedCode;
		std::string				m_ParsedLine;
		std::string				m_Line;
		std::string::iterator	m_LineIt;
		
		std::ifstream			m_File;

		bool ReadLine();
		void Parse();

	public:
		Preparser(Engine & engine, Script & script, std::string path) : 
			m_Engine(engine), m_Script(script), m_File(path, std::ios::in), m_State(init)
		{
			ASSERT(m_File.is_open());
			if (!m_File.is_open()) throw("failed to open file");
		}

		inline bool IsFinished()				{ return m_State == finished; }
		void Run();
		void Reset();
		
		static char GetTrigraphChar(char c);
	};

	class PreparserTask : public Task, public Preparser
	{
	public:
		PreparserTask(Engine & engine, Script & script, const std::string & path) : Preparser(engine, script, path)
		{
		}

	protected:
		bool IsTaskFinished() final override	{ return Preparser::IsFinished(); }
		void RunTask() final override			{ Preparser::Run(); }
		void ResetTask() final override			{ Preparser::Reset(); }
	};
}