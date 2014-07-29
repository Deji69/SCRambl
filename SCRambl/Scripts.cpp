#include "stdafx.h"
#include "Parser.h"
#include "Scripts.h"
#include "Directives.h"
#include "Literals.h"
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

			int col = 1;

			if (!code.empty())
			{
				for (auto it = code.begin(); it != code.end(); ++col)
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

					line.Append(c);
					eol = true;
				}
			}

			if (eol)
			{
				line.Append(Symbol::eol);
				eol = false;
			}

			dest.AddLine(line);
		}
	}

	void Script::LoadFile(const std::string & path)
	{
		if (!m_Code.IsEmpty()) m_Code.Clear();
		
		m_File = std::make_shared<File>(path, m_Code);

		Init();
	}

	Script::Position Script::Include(Script::Position & pos, const std::string & path)
	{
		ASSERT(m_File);
		try {
			auto file = m_File->IncludeFile(pos, path);
		}
		catch (const Script::File & file) {
			return m_Code.End();
		}
		return pos;

		/*
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
		return pos;*/
	}

	Script::Script(const CodeList & code) : m_Code(code)
	{ }

	Script::Position & Script::Code::Insert(Position & at, const Code & code)
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

	Script::Position & Script::Code::Insert(Position & at, const CodeLine & code)
	{
		// keep a count of how many characters are added so we can fix the pointer
		int i = 0;

		for (auto ch : code)
		{
			at << ch;
			++at;
			++i;
		}

		// revert to the old position
		at.m_CodeIt -= i;

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
				auto & symbols = beg.GetLineCode();
				auto beg_it = beg.GetSymbolIt();
				beg.NextLine();
				symbols.Erase(beg_it, symbols.End());
			} while (beg.GetLineIt() != end.GetLineIt());
		}
		
		// bye-bye
		end.m_CodeIt = beg.GetLineCode().Erase(beg.GetSymbolIt(), end.GetSymbolIt());
		if (end.m_CodeIt == end.GetLineCode().End())
			end.NextLine();

		// return the updated beginning position
		return beg = end;
	}

	std::string Script::Code::Select(const Position & beg, const Position & end) const
	{
		std::string r;
		for (auto cur = beg; cur != end; ++cur)
			r += *cur;
		return r;
	}

	CodeLine & Script::Code::Copy(const Position & beg, const Position & end, CodeLine & vec) const
	{
		for (auto cur = beg; cur != end; ++cur)
			vec.Append(*cur);
		return vec;
	}

	Script::Code::Code()
	{ }
	Script::Code::Code(const CodeList & code) : m_Code(code)
	{ }

	void Script::File::ReadFile(std::ifstream & file)
	{
		std::string code;

		// use this flag to prevent multiple concurrent eol's and escape new lines
		bool eol = false;

		m_Code.SetFile(this);

		while (std::getline(file, code))
		{
			int col = 1;
			CodeLine line;

			if (!code.empty())
			{
				for (auto it = code.begin(); it != code.end(); ++col)
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

					line.Append(c);
					eol = true;
				}
			}

			if (eol)
			{
				line.Append(Symbol::eol);
				eol = false;
			}

			m_End << line;
			if (!m_NumLines) m_Begin = m_End;
			m_End.NextLine();
			++m_NumLines;
		}

		m_Code.SetFile(m_Parent);
	}

	Script::File & Script::File::IncludeFile(Position & pos, const std::string & path)
	{
		m_Includes.emplace_back(path, m_Code, pos, this);
		pos.NextLine();
		return m_Includes.back();
	}

	Script::File::File(std::string path, Code & code): m_Code(code),
		m_Begin(code.End()), m_End(code.End()),
		m_Path(path),
		m_Parent(nullptr),
		m_NumLines(0)
	{
		std::ifstream file(path, std::ios::in);
		if (file) ReadFile(file);
		else throw *this;
		code.SetFile(this);
	}

	Script::File::File(std::string path, Code & code, Position pos, const File * parent): m_Code(code),
		m_Begin(pos.NextLine()), m_End(pos),
		m_Path(path),
		m_Parent(parent),
		m_NumLines(0)
	{
		std::ifstream file(path, std::ios::in);
		if (file) ReadFile(file);
		else throw *this;
	}

	void Script::Position::GetCodeLine()
	{
		if (!m_pCode->IsEmpty() && m_LineIt != m_pCode->GetLines().end())
		{
			m_CodeIt = m_LineIt->GetCode().Begin();
		}
	}

	Script::Position & Script::Position::NextLine()
	{
		if (m_LineIt != m_pCode->GetLines().end())
		{
			while (++m_LineIt != m_pCode->GetLines().end())
			{
				m_CodeIt = m_LineIt->GetCode().Begin();
				if (m_CodeIt != m_LineIt->GetCode().End())
					break;
			}
		}
		return *this;
	}

	bool Script::Position::Forward()
	{
		if (m_CodeIt != m_LineIt->GetCode().End())
		{
			if (++m_CodeIt != m_LineIt->GetCode().End())
				return true;
		}

		// better luck next line
		return NextLine();
	}

	bool Script::Position::Backward()
	{
		if (m_CodeIt != m_LineIt->GetCode().Begin())
		{
			--m_CodeIt;
			return true;
		}

		while (m_LineIt != m_pCode->GetLines().begin())
		{
			--m_LineIt;
			m_CodeIt = GetLine().GetCode().End();
			if (m_CodeIt != GetLine().GetCode().Begin())
				return true;
		}
		return false;
	}

	Script::Position & Script::Position::Insert(const Symbol & sym)
	{
		// insert a single character
		if (!IsEnd()) m_CodeIt = GetLine().GetCode().Insert(m_CodeIt, sym);
		return *this;
	}

	Script::Position & Script::Position::Delete()
	{
		// erase a single character
		if (!IsEnd())
		{
			m_CodeIt = GetLine().GetCode().Erase(m_CodeIt);
			if (m_CodeIt == GetLine().GetCode().End())
				Forward();
		}
		return *this;
	}

	Script::Position::Position()
	{ }
	Script::Position::Position(Code & code) : m_pCode(&code)
	{
		m_LineIt = m_pCode->GetLines().begin();
		GetCodeLine();
	}
	Script::Position::Position(Code & code, CodeList::iterator & it) : m_pCode(&code)
	{
		m_LineIt = it;
		GetCodeLine();
	}
}