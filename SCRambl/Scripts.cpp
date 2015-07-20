#include "stdafx.h"
#include "Parser.h"
#include "Scripts.h"
#include "Directives.h"
#include "Literals.h"
#include "Symbols.h"
#include "Tokens.h"

namespace SCRambl
{
	using namespace Scripts;

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

	/* LScript */
	//LScript::LScript() {}

	/* Script */
	Script::Script() //: m_LScript(&m_LScriptMain)
	{ }
	void Script::SetCode(const void * data) {
		const char * cdata = static_cast<const char*>(data);
		int i = 0;
		bool eol = false;
		
		if (!m_Code) m_Code = new Code();
		if (!m_Code->IsEmpty()) m_Code->Clear();
		if (cdata[i]) {
			do {
				CodeLine line;
				std::string code;
				for (; cdata[i] && cdata[i] != '\n'; ++i)
					code.push_back(cdata[i]);
				eol = ProcessCodeLine(code, line, eol);
				m_Code->AddLine(line);
			} while (cdata[++i]);
		}
	}
	void Script::ReadFile(std::ifstream & file, Code & dest) {
		std::string code;

		// use this flag to prevent multiple concurrent eol's and escape new lines
		bool eol = false;
		while (std::getline(file, code)) {
			CodeLine line;
			eol = ProcessCodeLine(code, line, eol);
			dest.AddLine(line);
		}
	}
	bool Script::ProcessCodeLine(const std::string& code, CodeLine& line, bool eol) {
		int col = 1;
		if (!code.empty()) {
			for (auto it = code.begin(); it != code.end(); ++col) {
				char c = *it;
				++it;

				switch (c)
				{
				case '?':
					if (it == code.end()) break;
					if (*it != '?') break;
					if (++it == code.end()) continue;
					if (auto t = GetTrigraphChar(*it)) {
						++it;
						c = t;
					} else {
						--it;
						break;
					}

					if (c == '\\') {
				case '\\':
					if (it == code.end()) {
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

		if (eol) {
			line.Append(Symbol::eol);
			eol = false;
		}
		return eol;
	}
	FileRef Script::OpenFile(const std::string& path) {
		FileRef ref(m_Files);
		bool wasempty = m_Files.empty();
		m_Files.emplace_back(m_Code ? (m_Code, path) : path);
		if (wasempty && ref) m_Code = ref->GetCode();
		if (m_Code) m_Code->SetFile(ref);
		return m_File = ref;
	}
	bool Script::IsFileOpen() const {
		return m_File && m_File->IsOpen();
	}
	long long Script::GetNumLines() const {
		return m_Code->NumLines();
	}
	Position Script::Include(Position & pos, const std::string & path) {
		ASSERT(m_File);
		try {
			m_File->IncludeFile(pos, path);
		}
		catch (const File &) {
			return m_Code->End();
		}
		return pos;
	}

	Tokens::Map Script::GenerateTokenMap() {
		Tokens::Map map;
		decltype(map.AddLine(0)) line = nullptr;
		int nline = -1;
		for (auto ptr = m_Tokens.Begin(); ptr != m_Tokens.End(); ++ptr) {
			auto curr_line = ptr->GetPosition().GetLine();

			if (nline == -1 || curr_line != nline)
				line = map.AddLine(nline = curr_line);

			line->AddToken(ptr.Ptr());
		}
		return map;
	}

	/* Scripts::File */
	File::File() : m_Code(new Code())
	{ }
	File::File(const File& file) : m_Parent(file.m_Parent), m_Code(file.m_Code),
		m_FileOpen(file.m_FileOpen), m_Begin(file.m_Begin), m_End(file.m_End),
		m_Path(file.m_Path), m_NumLines(file.m_NumLines)
	{
		for (auto& inc : file.m_Includes) {
			m_Includes.emplace_back(inc);
			m_Includes.back().m_Parent = this;
		}
	}
	File::File(File* parent) : m_Parent(parent), m_Code(parent->GetCode())
	{ }
	File::File(Code* code) : m_Code(code)
	{ }
	File::File(std::string path) : m_Code(new Code()) {
		Open(path);
	}
	File::File(File* parent, std::string path) : m_Parent(parent), m_Path(path), m_Code(parent->GetCode()), m_FileOpen(false) {
		Open(path);
	}
	File::File(Code* code, std::string path) : m_Code(code), m_Path(path), m_FileOpen(false) {
		Open(path);
	}
	File::~File() {
		if (m_Code) delete m_Code;
		m_Code = nullptr;
	}
	bool File::IsOpen() const {
		return m_FileOpen;
	}
	long File::GetNumLines() const {
		return m_NumLines;
	}
	long File::GetNumIncludes() const {
		return m_Includes.size();
	}
	Code* File::GetCode() const {
		return m_Code;
	}
	void File::SetCode(Code* code) {
		m_Code = code;
	}
	bool File::Open(std::string path) {
		std::ifstream file(path, std::ios::in);
		if (file) {
			m_FileOpen = true;
			m_Path = path;
			ReadFile(file);
		}
		else {
			m_FileOpen = false;
			m_Path = "";
		}
		return m_FileOpen;
	}
	void File::ReadFile(std::ifstream& file) {
		bool eol = false;	// use to prevent eol sequences and escape new lines
		std::string code;
		while (std::getline(file, code)) {
			int col = 1;
			CodeLine line;
			if (!code.empty()) {
				for (auto it = code.begin(); it != code.end(); ++col) {
					char c = *it;
					++it;
					
					if (c == '\\') {
						if (it == code.end()) {
							eol = false;
							continue;
						}
					}

					line.Append(c);
					eol = true;
				}
			}

			if (eol) {
				line.Append(Symbol::eol);
				eol = false;
			}

			m_Code->End() << line;
			m_Code->End().NextLine();
			++m_NumLines;
		}
	}
	FileRef File::IncludeFile(Position& pos, std::string path) {
		FileRef ref(m_Includes);
		m_Includes.emplace_back(this, path);
		pos.NextLine();
		return ref;
	}

	/* Scripts::Position */
	Position::Position() : m_pCode(nullptr)
	{ }
	Position::Position(Code & code) : Position(&code)
	{ }
	Position::Position(Code * code) : m_pCode(code)
	{
		m_LineIt = m_pCode->GetLines().begin();
		GetCodeLine();
	}
	Position::Position(Code * code, CodeList::iterator & it) : m_pCode(code)
	{
		m_LineIt = it;
		GetCodeLine();
	}
	Position::Position(Code * code, unsigned long line, unsigned long col) : m_pCode(code)
	{
		auto src_it = code->GetLines().end();
		for (auto it = code->GetLines().begin(); it != code->GetLines().end(); ++it) {
			if (it->GetLine() < line) {
				src_it = it;
			}
			//m_LineIt = m_pCode->AddLine();
		}
		
		m_LineIt = src_it != code->GetLines().end() ? src_it : code->GetLines().end();
		if (col != -1) {
			m_CodeIt = m_LineIt->GetCode().End();
			for (auto it = m_LineIt->GetCode().Begin(); it != m_LineIt->GetCode().End(); ++it)
			{
				if (it->Number() < col) {
					m_CodeIt = it;
				}
				else break;
			}
		}
		else m_CodeIt = m_LineIt->GetCode().Begin();
	}
	void Position::GetCodeLine() {
		if (!m_pCode->IsEmpty() && m_LineIt != m_pCode->GetLines().end())
			m_CodeIt = m_LineIt->GetCode().Begin();
	}
	Position & Position::NextLine() {
		if (m_LineIt != m_pCode->GetLines().end()) {
			while (++m_LineIt != m_pCode->GetLines().end()) {
				m_CodeIt = m_LineIt->GetCode().Begin();
				if (m_CodeIt != m_LineIt->GetCode().End())
					break;
			}
		}
		return *this;
	}
	bool Position::Forward()
	{
		if (m_CodeIt != m_LineIt->GetCode().End()) {
			if (++m_CodeIt != m_LineIt->GetCode().End())
				return true;
		}
		// better luck next line
		return NextLine();
	}
	bool Position::Backward()
	{
		if (m_CodeIt != m_LineIt->GetCode().Begin()) {
			--m_CodeIt;
			return true;
		}
		while (m_LineIt != m_pCode->GetLines().begin()) {
			--m_LineIt;
			m_CodeIt = GetLine().GetCode().End();
			if (m_CodeIt != GetLine().GetCode().Begin())
				return true;
		}
		return false;
	}
	Position & Position::Insert(const Symbol & sym) {
		// insert a single character
		if (!IsEnd()) m_CodeIt = GetLine().GetCode().Insert(m_CodeIt, sym);
		return *this;
	}
	Position & Position::Delete() {
		// erase a single character
		if (!IsEnd()) {
			m_CodeIt = GetLine().GetCode().Erase(m_CodeIt);
			if (m_CodeIt == GetLine().GetCode().End())
				Forward();
		}
		return *this;
	}

	/* Scripts::Code */
	Code::Code() = default;
	Code::Code(const CodeList& code) : m_Code(code)
	{ }
	CodeList& Code::operator*() {
		return GetLines();
	}
	CodeList* Code::operator->() {
		return &**this;
	}
	void Code::SetFile(FileRef file) {
		m_CurrentFile = file;
	}
	const CodeList& Code::GetLines() const {
		return m_Code;
	}
	CodeList& Code::GetLines() {
		return m_Code;
	}
	long Code::NumSymbols() const {
		return m_NumSymbols;
	}
	long Code::NumLines() const {
		return m_CurrentFile ? m_CurrentFile->GetNumLines() : m_Code.size();
	}
	bool Code::IsEmpty() const {
		return m_Code.empty();
	}
	const CodeList& Code::operator*() const {
		return GetLines();
	}
	const CodeList* Code::operator->() const {
		return &**this;
	}
	Position Code::Begin() {
		return this;
	}
	Position Code::End() {
		Position pos(this);
		pos.m_LineIt = m_Code.empty() ? m_Code.begin() : m_Code.end();
		pos.GetCodeLine();
		return pos;
	}
	void Code::AddLine(const CodeLine& code) {
		if (!code.Empty()) {
			m_Code.emplace_back(NumLines() + 1, code, m_CurrentFile);
			m_NumSymbols += code.Size();
		}
	}
	Position& Code::AddLine(Position& pos, const CodeLine& code) {
		if (!code.Empty()) {
			pos.m_LineIt = m_Code.emplace(pos.GetLineIt(), NumLines() + 1, code, m_CurrentFile);
			pos.GetCodeLine();
			m_NumSymbols += code.Size();
		}
		return pos;
	}
	void Code::Clear() {
		m_Code.clear();
		m_NumSymbols = 0;
	}
	Position& Code::Insert(Position& at, const Code& code) {
		auto lnit = at.GetLineIt();
		++lnit;
		for (auto line : *code) {
			lnit = at.GetCode()->GetLines().insert(lnit, line);
			++lnit;
		}
		at.NextLine();
		return at;
	}
	Position& Code::Insert(Position& at, const CodeLine& code) {
		// keep a count of how many characters are added so we can fix the pointer
		int i = 0;
		for (auto ch : code) {
			at << ch;
			++at;
			++i;
		}

		// revert to the old position
		at.m_CodeIt -= i;
		return at;
	}
	Position& Code::Erase(Position& beg, Position& end) {
		// update the end position
		auto end_it = end.GetSymbolIt();
		auto end_ln = end.GetLineIt();

		// if they're not on the same line, erase the inbetweeny lines
		if (beg.GetLineIt() != end.GetLineIt()) {
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
	std::string Code::Select(const Position& beg, const Position& end) const {
		std::string r;
		for (auto cur = beg; cur && cur != end; ++cur)
			if (cur) r += *cur;
		return r;
	}
	CodeLine& Code::Copy(const Position& beg, const Position& end, CodeLine& vec) const {
		for (auto cur = beg; cur != end; ++cur)
			vec.Append(*cur);
		return vec;
	}

	/* Scripts::Line */
	Line::Line(unsigned long line, CodeLine code, const FileRef file) : m_Line(line), m_Code(code), m_File(file)
	{ }
	const FileRef Line::GetFile() const {
		return m_File;
	}
	CodeLine& Line::GetCode() {
		return m_Code;
	}
	const CodeLine& Line::GetCode() const {
		return m_Code;
	}
	unsigned long Line::GetLine() const {
		return m_Line;
	}
	Line::operator const CodeLine &() const {
		return GetCode();
	}
	Line::operator CodeLine &() {
		return GetCode();
	}
	Line::operator unsigned long() const {
		return GetLine();
	}
}