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
#include <unordered_map>
#include <memory>
#include "utils.h"
#include "Symbols.h"

namespace SCRambl
{
	namespace Scripts
	{
		class Code;
		class File;
		class Line;
		class Column;
		class Range;
		class Position;
		class Label;

		using CodeList = std::list<Line>;
		using Files = std::vector<File>;
		using FileRef = VecRef<File>;

		/*\ Scripts::Line - this is one \*/
		class Line
		{
		public:
			Line() = default;
			Line(unsigned long, CodeLine, const FileRef);

			unsigned long GetLine() const;
			CodeLine& GetCode();
			const CodeLine& GetCode() const;
			const FileRef GetFile() const;

			operator const CodeLine&() const;
			operator CodeLine&();
			operator unsigned long() const;

		private:
			const FileRef m_File;
			unsigned long m_Line = 0;
			CodeLine m_Code;
		};

		/*\ Scripts::Code - where symbolic data lives in peaceful bliss \*/
		class Code
		{
			friend class Position;

		public:
			Code();
			Code(const CodeList&);

			void SetFile(FileRef file);
			const CodeList & GetLines() const;
			long NumSymbols() const;
			long NumLines() const;
			bool IsEmpty() const;

			const CodeList& operator *() const;
			const CodeList* operator ->() const;

			/*\ Get begin line Position \*/
			Position Begin();

			/*\ Get end line Position \*/
			Position End();

			/*\ Add a line of code to the grande list \*/
			void AddLine(const CodeLine&);
			Position& AddLine(Position&, const CodeLine&);

			/*\ Make the code vanish completely \*/
			void Clear();

			/*\
			 - Insert code from elsewhere onto the next line
		 	 - Returns the beginning position of the inserted code
			\*/
			Position& Insert(Position&, const Code&);

			/*\
			 - Insert code symbols into the line at Position
			 - Returns the beginning position of the inserted code
			\*/
			Position& Insert(Position&, const CodeLine&);

			/*\
			 - Erase the code from Position A to (and including) Position B
			 - Updates all positions automatically and returns the reference to Position A
			\*/
			Position& Erase(Position&, Position&);

			/*\
			 - Selects a string sequence from Position A to Position B
			\*/
			std::string Select(const Position&, const Position&) const;

			/*\
			 - Copy the symbols from Position A to Position B
			 - Returns the vector of collected symbols
			\*/
			CodeLine& Copy(const Position&, const Position&, CodeLine&) const;

		private:
			CodeList m_Code;
			FileRef m_CurrentFile;
			long m_NumSymbols = 0;

			CodeList& GetLines();
			CodeList& operator*();
			CodeList* operator->();
		};

		/*\ Scripts::Position(tm) - iterating through all that matters since '14 \*/
		class Position
		{
			friend class Code;

		public:
			// construct invalid thing
			Position();

			// beginning of code
			Position(Code &);
			Position(Code *);
			// specified line of code
			Position(Code *, unsigned long, unsigned long = 0);
			Position(Code *, CodeList::iterator &);

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
			Position& Delete();

			/*\
			 - Attempt to insert a symbol at the current position
		 	 - Returns a reference to this position at the inserted symbol
			\*/
			Position& Insert(const Symbol&);

			/*\
			 - Attempt to insert multiple symbols at the current position
			 - Returns a reference to this position at the inserted symbol
			\*/
			Position& Insert(const CodeLine&);

			/*\
			- Select a string from this position to the specified one
			\*/
			inline std::string Select(const Position& end) const {
				return GetCode()->Select(*this, end);
			}

			/*\ Returns true if this position is at the end of the symbol list \*/
			inline bool IsEnd() const {
				return !m_pCode || GetCode()->IsEmpty() || m_LineIt == GetCodeLines().end() || m_CodeIt == m_LineIt->GetCode().End();
			}

			/*\ Returns true if each Position refers to the same script position \*/
			inline bool Compare(const Position& pos) const {
				return m_pCode == pos.m_pCode && m_LineIt == pos.m_LineIt && m_CodeIt == pos.m_CodeIt;
			}

			/*\ Returns true if both Position's are on the same line \*/
			inline bool IsOnSameLine(const Position& pos) const {
				return m_LineIt == pos.m_LineIt;
			}

			/*\ Returns true if the Position is on an earlier line \*/
			inline bool IsOnEarlierLine(const Position& pos) const {
				return GetLine().GetLine() > pos.GetLine().GetLine();
			}

			/*\ Returns true if the position is on a later line \*/
			inline bool IsOnLaterLine(const Position& pos) const {
				return GetLine().GetLine() < pos.GetLine().GetLine();
			}

			/*\ Returns true if the position is earlier \*/
			inline bool IsEarlier(const Position& pos) const {
				return IsOnEarlierLine(pos) || (IsOnSameLine(pos) && GetColumn() > pos.GetColumn());
			}

			/*\ Returns true if the position is later \*/
			inline bool IsLater(const Position& pos) const {
				return IsOnLaterLine(pos) || (IsOnSameLine(pos) && GetColumn() < pos.GetColumn());
			}

			/*\ Returns true if the Position points to the same character \*/
			inline bool Compare(const char c) const {
				return !IsEnd() ? *m_CodeIt == c : false;
			}

			// Get teh code
			inline Code* GetCode() { ASSERT(m_pCode); return m_pCode; }
			inline const Code* GetCode() const { ASSERT(m_pCode); return m_pCode; }
			inline CodeList& GetCodeLines() { ASSERT(m_pCode); return m_pCode->GetLines(); }
			inline const CodeList& GetCodeLines() const { ASSERT(m_pCode); return m_pCode->GetLines(); }

			// Get the current line of this position
			inline Line& GetLine() { return *m_LineIt; }
			inline const Line& GetLine() const { return *m_LineIt; }

			// Get the current number of the column at this position
			inline int GetColumn() const { return m_CodeIt->Number(); }

			// Get the current line code of this position
			inline CodeLine& GetLineCode() { return m_LineIt->GetCode(); }
			inline const CodeLine& GetLineCode() const { return m_LineIt->GetCode(); }

			// Get the current symbol of this position
			inline Symbol& GetSymbol() { return *m_CodeIt; }
			inline const Symbol& GetSymbol() const { return *m_CodeIt; }

			// GetSymbol()
			inline Symbol& operator*() { return GetSymbol(); }
			inline const Symbol& operator*() const { return GetSymbol(); }

			// GetSymbol()
			inline Symbol* operator->() { return &**this; }
			inline const Symbol* operator->() const { return &**this; }

			// !IsEnd()
			inline operator bool() const { return !IsEnd(); }

			// Insert Symbol
			inline Position& operator<<(const Symbol& sym) {
				return Insert(sym);
			}

			// Insert Line
			inline Position& operator<<(const CodeLine& line) {
				m_pCode->AddLine(*this, line);
				return *this;
			}

			// 
			inline Position& operator=(const Position& pos) {
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
			inline Position& operator++() {
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
			inline Position& operator+=(int n) {
				for (n; n > 0; --n) if (!Forward()) break;
				return *this;
			}

			// Backward()
			inline Position& operator--() {
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
			inline Position& operator-=(int n) {
				for (n; n > 0; --n) if (!Backward()) break;
				return *this;
			}

			// Compare()
			inline bool operator==(const Position& pos) const {
				return Compare(pos);
			}
			inline bool operator!=(const Position& pos) const {
				return !(*this == pos);
			}

			// IsLater()
			inline bool operator<(const Position& pos) const {
				return IsLater(pos);
			}
			// IsEarlier()
			inline bool operator>(const Position& pos) const {
				return IsEarlier(pos);
			}

			// Compare() || IsLater()
			inline bool operator<=(const Position& pos) const {
				return *this == pos || *this < pos;
			}
			// Compare() || IsEarlier()
			inline bool operator>=(const Position& pos) const {
				return *this == pos || *this > pos;
			}

			// CompareChar()
			inline bool operator==(const char c) const {
				return Compare(c);
			}
			inline bool operator!=(const char c) const {
				return !(*this == c);
			}

			// String formatter
			static inline std::string Formatter(const Position& pos) {
				return std::to_string(pos.GetLine().GetLine());
			}
		
		private:
			Code* m_pCode = nullptr;
			CodeList::iterator m_LineIt;		// (x)
			CodeLine::iterator m_CodeIt;		// (y)

			inline CodeList::iterator GetLineIt() { return m_LineIt; }
			inline CodeLine::iterator GetSymbolIt() { return m_CodeIt; }

			void GetCodeLine();
		};

		/*\ Scripts::Range - Range of Position's in the script \*/
		class Range
		{
			std::pair<Position, Position>	m_Pair;

		public:
			Range() = default;
			Range(const Position &a, const Position &b) : m_Pair(a <= b ? std::make_pair(a, b) : std::make_pair(b, a))
			{ }
			/*Range(Position a, Position b) : m_Pair(a < b ? std::make_pair(a, b) : std::make_pair(b, a))
			{ }*/
			Range(std::pair<Position, Position> pair) : m_Pair(pair.first <= pair.second ? std::make_pair(pair.first, pair.second) : std::make_pair(pair.second, pair.first))
			{ }

			// String formatter
			static inline std::string Formatter(const Range& range) {
				return range.Begin().Select(range.End());
			}

			inline operator const std::pair<Position, Position>() const	{ return Get(); }
			inline const std::pair<Position, Position>& Get() const { return m_Pair; }
			inline const Position& Begin() const { return m_Pair.first; }
			inline const Position& begin() const { return Begin(); }
			inline const Position& End() const { return m_Pair.second; }
			inline const Position& end() const { return End(); }
			inline std::string Format() const { return Formatter(*this); }
		};
	}
}