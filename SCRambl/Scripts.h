/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <assert.h>
#include "utils.h"
//#include "FileSystem.h"
#include "Errors.h"
#include "Symbols.h"
#include "Tokens.h"

namespace SCRambl
{
	//typedef std::list<class Script::Line> CodeList;
	typedef std::vector<std::string> StringList;

	class Script
	{
	public:
		class Code;
		class File;
		class Line;
		class Column;
		class Range;
		class Position;

		typedef std::list<Line> CodeList;
		typedef std::vector<File> Files;

		/*\
		 - Script::Line - this is one
		\*/
		class Line
		{
			const File		*	m_File = nullptr;
			long				m_Line = 0;
			CodeLine			m_Code;

		public:
			Line() = default;
			Line(long line, CodeLine code, const File * file) : m_Line(line), m_Code(code), m_File(file)
			{ }

			inline const File * GetFile() const			{ return m_File; }
			inline CodeLine & GetCode()					{ return m_Code; }
			inline const CodeLine & GetCode() const		{ return GetCode(); }

			inline long GetLine() const					{ return m_Line; }

			inline operator const CodeLine &() const	{ return GetCode(); }
			inline operator CodeLine &()				{ return GetCode(); }
			inline operator long() const				{ return GetLine(); }
		};

		/*\
		 - Script::Code - where symbolic data lives in peaceful bliss
		\*/
		class Code
		{
			friend class Script::Position;

			CodeList				m_Code;
			const File			*	m_CurrentFile;
			long					m_NumSymbols = 0;
			//long					m_NumLines = 0;

			inline CodeList	& GetLines()		{ return m_Code; }
			inline CodeList & operator *()		{ return GetLines(); }
			inline CodeList * operator ->()		{ return &**this; }

		public:
			Code();
			Code(const CodeList & code);

			inline void	SetFile(const File * file)			{ m_CurrentFile = file; }
			inline const CodeList	&	GetLines() const	{ return m_Code; }
			inline long 				NumSymbols() const	{ return m_NumSymbols; }
			inline long 				NumLines() const	{ return /*m_NumLines*/m_CurrentFile->GetNumLines(); }
			inline bool					IsEmpty() const		{ return m_Code.empty(); }

			inline const CodeList & operator *() const		{ return GetLines(); }
			inline const CodeList * operator ->() const		{ return &**this; }

			/*\
			 - Get begin line Position
			\*/
			inline Position Begin() {
				return{ *this };
			}

			/*\
			 - Get end line Position
			\*/
			inline Position End() {
				Position pos{ *this };
				pos.m_LineIt = m_Code.empty() ? m_Code.begin() : m_Code.end();
				pos.GetCodeLine();
				return pos;
			}

			/*\
			 - Add a line of code to the grande list
			\*/
			inline void AddLine(const CodeLine & code) {
				if (!code.Empty())
				{
					m_Code.emplace_back(NumLines() + 1, code, m_CurrentFile);
					m_NumSymbols += code.Size();
				}
			}
			inline Position & AddLine(Position & pos, const CodeLine & code) {
				if (!code.Empty())
				{
					pos.m_LineIt = m_Code.emplace(pos.GetLineIt(), NumLines() + 1, code, m_CurrentFile);
					pos.GetCodeLine();
					m_NumSymbols += code.Size();
				}
				return pos;
			}

			/*\
			 - Make the code vanish completely
			\*/
			inline void Clear() {
				m_Code.clear();
				//m_NumLines = 0;
				m_NumSymbols = 0;
			}

			/*\
			 - Insert code from elsewhere onto the next line
			 - Returns the beginning position of the inserted code
			\*/
			Position & Insert(Position &, const Code &);

			/*\
			- Insert code symbols into the line at Position
			- Returns the beginning position of the inserted code
			\*/
			Position & Insert(Position &, const CodeLine/*::vector*/ &);
			//inline Position & Insert(Position & pos, const CodeLine & line)		{ return Insert(pos, line.Symbols()); }

			/*\
			 - Erase the code from Position A to (and including) Position B
			 - Updates all positions automatically and returns the reference to Position A
			\*/
			Position & Erase(Position &, Position &);

			/*\
			 - Selects a string sequence from Position A to Position B
			\*/
			std::string Select(const Position &, const Position &) const;

			/*\
			 - Copy the symbols from Position A to Position B
			 - Returns the vector of collected symbols
			\*/
			CodeLine & Copy(const Position &, const Position &, CodeLine &) const;
		};

		/*\
		 - Script::Position(tm) - iterating through all that matters since '14
		\*/
		class Position
		{
			friend class Code;

			Code					*	m_pCode;
			CodeList::iterator			m_LineIt;		// (x)
			CodeLine::iterator			m_CodeIt;		// (y)

			inline CodeList::iterator		GetLineIt()			{ return m_LineIt; }
			inline CodeLine::iterator		GetSymbolIt()		{ return m_CodeIt; }

			void GetCodeLine();

		public:
			// construct invalid thing
			Position();
			// beginning of code
			Position(Code &);
			// specified line of code
			Position(Code &, CodeList::iterator &);

			/*\
			- Attempt to set this position at the next line
			\*/
			Position & NextLine();

			/*\
			 - Attempt to set this position at the next symbol
			 - Returns true if another symbol is available
			\*/
			bool Forward();

			/*\
			 - Attempt to set this position at the previous symbol
			 - Returns true if another symbol is available
			\*/
			bool Backward();

			/*\
			 - Attempt to erase the symbol at the current position
			 - Returns a reference to this position at the next symbol
			\*/
			Position & Delete();

			/*\
			 - Attempt to insert a symbol at the current position
			 - Returns a reference to this position at the inserted symbol
			\*/
			Position & Insert(const Symbol &);

			/*\
			 - Attempt to insert multiple symbols at the current position
			 - Returns a reference to this position at the inserted symbol
			\*/
			Position & Insert(const CodeLine &);

			/*\
			 - Select a string from this position to the specified one
			\*/
			inline std::string Select(const Position & end) const {
				return GetCode().Select(*this, end);
			}

			/*\
			 - Returns true if this position is at the end of the symbol list
			\*/
			inline bool IsEnd() const {
				return !m_pCode || GetCode().IsEmpty() || m_LineIt == GetCode()->end() || m_CodeIt == m_LineIt->GetCode().End();
			}

			/*\
			 - Returns true if each Position refers to the same script position
			\*/
			inline bool Compare(const Position & pos) const {
				return m_pCode == pos.m_pCode && m_LineIt == pos.m_LineIt && m_CodeIt == pos.m_CodeIt;
			}

			/*\
			 - Returns true if the Position points to the same character
			\*/
			inline bool Compare(const char c) const {
				return !IsEnd() ? *m_CodeIt == c : false;
			}

			// Get teh code
			inline Code	& GetCode()						{ ASSERT(m_pCode); return *m_pCode; }
			inline const Code & GetCode() const			{ ASSERT(m_pCode); return *m_pCode; }

			// Get the current line of this position
			inline Line & GetLine()							{ return *m_LineIt; }
			inline const Line & GetLine()	const			{ return *m_LineIt; }

			// Get the current number of the column at this position
			inline int GetColumn() const					{ return m_CodeIt->Number(); }

			// Get the current line code of this position
			inline CodeLine & GetLineCode()					{ return m_LineIt->GetCode(); }
			inline const CodeLine & GetLineCode()	const	{ return m_LineIt->GetCode(); }

			// Get the current symbol of this position
			inline Symbol & GetSymbol()						{ return *m_CodeIt; }
			inline const Symbol & GetSymbol() const			{ return *m_CodeIt; }

			// GetSymbol()
			inline Symbol & operator*()					{ return GetSymbol(); }
			inline const Symbol & operator*() const		{ return GetSymbol(); }

			// GetSymbol()
			inline Symbol * operator->()				{ return &**this; }
			inline const Symbol * operator->() const	{ return &**this; }

			// !IsEnd()
			inline operator bool() const				{ return !IsEnd(); }

			// Insert Symbol
			inline Position & operator<<(const Symbol & sym) {
				return Insert(sym);
			}

			// Insert Line
			inline Position & operator<<(const CodeLine & line) {
				m_pCode->AddLine(*this, line);
				return *this;
			}

			// 
			inline Position & operator=(const Position & pos) {
				m_pCode = pos.m_pCode;
				m_CodeIt = pos.m_CodeIt;
				m_LineIt = pos.m_LineIt;
				return *this;
			}

			inline Position operator[](int i) {
				Position new_pos = *this;
				for (i; i > 0; --i) if (!new_pos.Forward()) break;
				return new_pos;
			}

			// Forward()
			inline Position & operator++() {
				Forward();
				return *this;
			}
			inline Position operator++(int) {
				auto pos = *this;
				Forward();
				return pos;
			}

			inline Position operator+(int n) const {
				Position new_pos = *this;
				for (n; n > 0; --n)
				{
					if (!new_pos.Forward()) break;
				}
				return new_pos;
			}

			inline Position & operator+=(int n) {
				for (n; n > 0; --n) if (!Forward()) break;
				return *this;
			}

			// Backward()
			inline Position & operator--() {
				Backward();
				return *this;
			}
			inline Position operator--(int) {
				auto pos = *this;
				Backward();
				return pos;
			}

			inline Position operator-(int n) const {
				Position new_pos = *this;
				for (n; n > 0; --n)
				{
					if (!new_pos.Backward()) break;
				}
				return new_pos;
			}

			inline Position & operator-=(int n) {
				for (n; n > 0; --n) if (!Backward()) break;
				return *this;
			}

			// Compare()
			inline bool operator==(const Position & pos) const {
				return Compare(pos);
			}
			inline bool operator!=(const Position & pos) const {
				return !(*this == pos);
			}

			// CompareChar()
			inline bool operator==(const char c) const {
				return Compare(c);
			}
			inline bool operator!=(const char c) const {
				return !(*this == c);
			}

			// String formatter
			static inline std::string Formatter(const Position & pos) {
				return std::to_string(pos.GetLine().GetLine());
			}
		};

		/*\
		 * Script::Range - Range of Position's in the script
		\*/
		class Range
		{
			std::pair<Position, Position>	m_Pair;

		public:
			Range(Position a, Position b) : m_Pair(std::make_pair(a, b))
			{
			}
			Range(std::pair<Position, Position> pair) : m_Pair(pair)
			{
			}

			inline operator const std::pair<Position, Position>() const	{ return Get(); }
			inline const std::pair<Position, Position> & Get() const	{ return m_Pair; }
			inline const Position & Begin() const		{ return m_Pair.first; }
			inline const Position & End() const			{ return m_Pair.second; }

			// String formatter
			static inline std::string Formatter(const Range & range) {
				return range.Begin().Select(range.End());
			}
		};

		/*\
		 * Script::File - Script files and includes
		\*/
		class File
		{
			const File		*	m_Parent = nullptr;
			Code			&	m_Code;					// code source
			Position			m_Begin;				// beginning of this file in code source
			Position			m_End;					// end of this file in code source
			Files				m_Includes;				// included files
			
			long				m_NumLines = 0;
			std::string			m_Path;

		public:
			File(std::string, Code &);
			File(std::string, Code &, Position, const File *);

			inline long GetNumLines() const { return m_NumLines; }
			inline const std::string & GetPath() const { return m_Path; }

			void ReadFile(std::ifstream &);
			File & IncludeFile(Position &, const std::string &);
		};
		
	private:
		std::shared_ptr<File>				m_File;
		Code								m_Code;
		Tokens								m_Tokens;
		StringList							m_Warnings;

		// Initialise script for parsing with current code
		void Init();

		void Error(int code, const std::string &);

		void ReadFile(std::ifstream &, Code &);

		// DEBUG
#ifdef _DEBUG
	public:
#endif
		void OutputFile() {
			std::ofstream file("script.txt");
			if (file)
			{
				for (Script::Position pos(m_Code); pos; ++pos)
				{
					char c = *pos;
					file << (c ? c : '\n');
				}
			}
			file.flush();
		}

	public:
		Script()
		{ }

		// Construct script parser with code from memory
		Script(const CodeList &);

		~Script()
		{ }

		// Load file into code lines
		void LoadFile(const std::string &);
		
		// Include file in specific code line
		Position & Include(Position &, const std::string &);

		// OK?
		inline bool OK() const				{ return true; }
		// Number of source lines
		inline size_t GetNumLines() const	{ return m_Code.NumLines(); }
		// Get the almighty source code list
		inline Code & GetCode()				{ return m_Code; }
		// Get the informative token list
		inline Tokens & GetTokens()			{ return m_Tokens; }
	};
}