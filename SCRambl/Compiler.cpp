#include "stdafx.h"
#include "Standard.h"
#include "Compiler.h"
#include "Exception.h"
#include "Scripts.h"
#include "TokensB.h"

#include <cctype>

namespace SCRambl
{
	namespace Compiler
	{
		void Compiler::Init()
		{
			m_Task(Event::Begin);
			m_State = compiling;
		}
		void Compiler::Reset()
		{
		}
		void Compiler::Run()
		{
			switch (m_State) {
			case init:
				Init();
				return;
			case compiling:
				/*if (m_TokenIt != m_Tokens.End())
					Parse();
				else
				{
					m_Task(Event::Finish);
					m_State = finished;
				}*/
				return;
			}
		}

		Compiler::Compiler(Task & task, Engine & engine, Script & script) :
			m_State(init), m_Task(task), m_Engine(engine), m_Script(script)
		{
		}
	}
}