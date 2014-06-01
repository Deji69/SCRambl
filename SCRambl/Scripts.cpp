#include "stdafx.h"
#include "Parser.h"
#include "Scripts.h"
#include "Directives.h"
#include "Literals.h"
#include "Expressions.h"
#include "Symbols.h"

namespace SCRambl
{
	void Script::Init()
	{
	}

	void Script::Error(int code, const std::string & msg)
	{
		//ScriptError()
	}

	// returns 0 on failure
	char GetTrigraphChar(char c)
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

	void Script::ReadFile(std::ifstream & file, Code & dest)
	{
		std::string code;

		// use this flag to prevent multiple concurrent eol's and escape new lines
		bool eol = false;

		while (std::getline(file, code))
		{
			CodeLine line;

			if (!code.empty())
			{
				for (auto it = code.begin(); it != code.end();)
				{
					char c = *it;
					++it;

					switch (c)
					{
					case '?':
						if (it == code.end()) break;
						if (*it != '?') break;
						if (++it == code.end()) continue;
						if (auto t = GetTrigraphChar(*it))
						{
							++it;
							c = t;
						}
						else
						{
							--it;
							break;
						}

						if (c == '\\')
						{
					case '\\':
						if (it == code.end())
						{
							eol = false;
							continue;
						}
						}
						break;
					}

					line.Symbols().emplace_back(c);
					eol = true;
				}
			}

			if (eol)
			{
				line.Symbols().emplace_back(Symbol::eol);
				eol = false;
			}

			dest.AddLine(line);
		}
	}

	void Script::LoadFile(const std::string & path)
	{
		if (m_Code.NumLines()) m_Code.Clear();
		
		std::ifstream file(path, std::ios::in);

		if (file.is_open()) ReadFile(file, m_Code);
		//else throw;

		//std::copy(std::istream_iterator<ScriptLine>(file), std::istream_iterator<ScriptLine>(), std::back_inserter(m_Code));

		Init();
	}

	Script::Position & Script::Include(Script::Position & pos, const std::string & path)
	{
		std::ifstream file(path, std::ios::in);
		if (file.is_open())
		{
			Code code;
			ReadFile(file, code);
			if (!code.IsEmpty())
			{
				pos = m_Code.Insert(pos, code);
			}
		}
		return pos;
	}

	Script::Script(const CodeList & code) : m_Code(code)
	{ }

	Script::Position & Script::Code::Insert(Position & at, Code & code)
	{
		auto lnit = at.GetLineIt();
		++lnit;
		for (auto line : *code)
		{
			lnit = at.GetCode()->insert(lnit, line);
			++lnit;
		}
		at.NextLine();
		return at;
	}

	Script::Position & Script::Code::Erase(Position & beg, Position & end)
	{
		// update the end position
		auto end_it = end.GetSymbolIt();
		auto end_ln = end.GetLineIt();

		// if they're not on the same line, erase the inbetweeny lines
		if (beg.GetLineIt() != end.GetLineIt())
		{
			do {
				auto & symbols = beg.GetLineCode().Symbols();
				auto beg_it = beg.GetSymbolIt();
				beg.NextLine();
				symbols.erase(beg_it, symbols.end());
			} while (beg.GetLineIt() != end.GetLineIt());
		}
		
		// bye-bye
		end.m_CodeIt = beg.GetLineCode().Symbols().erase(beg.GetSymbolIt(), end.GetSymbolIt());
		if (end.m_CodeIt == end.GetLineCode().Symbols().end())
			end.NextLine();

		// return the updated beginning position
		return beg = end;
	}

	std::string Script::Code::Select(Position beg, Position end) const
	{
		// so elegant, so simple
		std::string r;

		for (auto cur = beg; cur != end; ++cur)
			r += *cur;

		return r;
	}

	Script::Code::Code()
	{ }
	Script::Code::Code(const CodeList & code) : m_Code(code)
	{ }

	ScriptFile::ScriptFile(std::string path, CodeList & code, const ScriptFile * parent) :
		m_FilePath(path), m_Parent(parent), m_NumLines(0)
	{
		std::ifstream file(path, std::ios::in);
		if (file)
		{
			std::string line;
			while (std::getline(file, line))
			{
				//code.emplace_back(++m_NumLines, line, this);
			}
		}
	}

	void Script::Position::GetCodeLine()
	{
		if (m_LineIt != m_pCode->GetLines().end())
		{
			m_CodeIt = m_LineIt->GetCode().Symbols().begin();
		}
	}

	bool Script::Position::NextLine()
	{
		while (++m_LineIt != m_pCode->GetLines().end())
		{
			m_CodeIt = m_LineIt->GetCode().Symbols().begin();
			if (m_CodeIt != m_LineIt->GetCode().Symbols().end())
				return true;
		}
		return false;
	}

	bool Script::Position::Forward()
	{
		if (m_CodeIt != m_LineIt->GetCode().Symbols().end())
		{
			if (++m_CodeIt != m_LineIt->GetCode().Symbols().end())
				return true;
		}

		// better luck next line
		return NextLine();
	}

	bool Script::Position::Backward()
	{
		if (m_CodeIt != m_LineIt->GetCode().Symbols().begin())
		{
			--m_CodeIt;
			return true;
		}

		while (m_LineIt != m_pCode->GetLines().begin())
		{
			--m_LineIt;
			m_CodeIt = m_LineIt->GetCode().Symbols().end();
			if (m_CodeIt != m_LineIt->GetCode().Symbols().begin())
				return true;
		}
		return false;
	}

	Script::Position & Script::Position::Insert(Symbol sym)
	{
		// insert a single character
		if (!IsEnd()) m_CodeIt = m_LineIt->GetCode().Symbols().insert(m_CodeIt, sym);
		return *this;
	}

	Script::Position & Script::Position::Delete()
	{
		// erase a single character
		if (!IsEnd())
		{
			m_CodeIt = m_LineIt->GetCode().Symbols().erase(m_CodeIt);
			if (m_CodeIt == GetLine().GetCode().Symbols().end())
				Forward();
		}
		return *this;
	}

	Script::Position::Position()
	{ }

	Script::Position::Position(Code & code):
		m_pCode(&code)
	{
		m_LineIt = m_pCode->GetLines().begin();
		GetCodeLine();
	}
}