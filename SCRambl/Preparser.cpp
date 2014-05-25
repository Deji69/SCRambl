#include "stdafx.h"
#include "Preparser.h"
#include "Code.h"

namespace SCRambl
{
	bool Preparser::ReadLine()
	{
		while (std::getline(m_File, m_Line))
		{
			++m_NumLines;
			if (!m_Line.empty())
			{
				m_ParsedLine = "";
				m_LineIt = m_Line.begin();
				return true;
			}
		}
		return false;
	}

	void Preparser::Run()
	{
		switch (m_State)
		{
		case init:
			Reset();
			return;

		case reading:
			if (ReadLine()) m_State = parsing;
			else m_State = finished;
			return;

		case parsing:
			Parse();
			return;

		case end_of_line:
		case continue_next_line:
			if (!m_ParsedCode.Symbols().empty())
			{
				m_Script.Code().emplace_back(m_NumLines, m_ParsedCode, nullptr);		// TODO: something with ScriptFile
				m_ParsedCode.Symbols().clear();
			}
			m_State = reading;
			return;

		case finished:
			BREAK();
			return;
		}
	}

	void Preparser::Parse()
	{
		auto& symbols = m_ParsedCode.Symbols();

		while (true)
		{
			if (m_LineIt == m_Line.end())
			{
				symbols.emplace_back(Symbol::eol);
				m_State = end_of_line;
				return;
			}

			char c = *m_LineIt;
			++m_LineIt;

			switch (c)
			{
			case '?':
				if (m_LineIt == m_Line.end()) continue;
				if (*m_LineIt != '?') break;
				if (++m_LineIt == m_Line.end()) continue;
				if (c = GetTrigraphChar(*m_LineIt)) break;
				else --m_LineIt;
				break;

			case '\\':
				if (m_LineIt == m_Line.end())
				{
					m_State = continue_next_line;
					return;
				}
				break;
			}

			symbols.emplace_back(c);
		}
	}

	// returns 0 on failure
	char Preparser::GetTrigraphChar(char c)
	{
		switch (c)
		{
		case '=': return '#';
		case '\'': return '^';
		case '!': return '|';
		case '-': return '~';
		case '/': return '\\';
		case '(': return '[';
		case ')': return ']';
		case '<': return '{';
		case '>': return '}';
		}
		return 0;
	}

	void Preparser::Reset()
	{
		switch (m_State)
		{
		default:
			// mid-process reset

		case finished:
			// end-process reset
			break;

		case init:
			// begin-process init
			break;
		}

		m_State = reading;
		m_NumLines = 0;
	}
}