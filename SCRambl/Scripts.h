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
#include "FileSystem.h"
#include "Errors.h"
#include "Symbols.h"

namespace SCRambl
{
	class Script;

	typedef std::vector<class ScriptFile> ScriptFiles;
	typedef std::list<class ScriptLine> CodeList;
	typedef std::vector<std::string> StringList;

	class ScriptFile
	{
		const ScriptFile				*	m_Parent = nullptr;
		long								m_NumLines = 0;
		std::string							m_FilePath;
		ScriptFiles							m_Includes;

	public:
		ScriptFile(std::string path, CodeList & code, const ScriptFile * parent = nullptr);

		inline long GetNumLines() const { return m_NumLines; }
		inline const std::string & GetPath() const { return m_FilePath; }
	};

	class ScriptLine
	{
		const ScriptFile	*	File = nullptr;
		long					Line = -1;
		CodeLine				Code;

	public:
		ScriptLine(long line, CodeLine code, const ScriptFile * file) : Line(line), Code(code), File(file)
		{ }

		ScriptLine() { }

		inline const ScriptFile * GetFile() const	{ return File; }
		inline long GetLine() const					{ return Line; }
		inline CodeLine & GetCode()					{ return Code; }
		inline const CodeLine & GetCode() const		{ return Code; }

		operator const CodeLine &() const	{ return GetCode(); }
		operator CodeLine &()				{ return GetCode(); }
		operator long() const				{ return Line; }
	};

	class Script
	{
		enum eSCR_Error
		{
			SCR_UNKNOWN_ERROR = 1000,			// dummy. no, I mean you're a dummy
			SCR_BAD_DIRECTIVE_CHAR = 1001,
			SCR_BAD_CONSTEXPR_OPERATOR = 1002,
			SCR_BAD_NUMBER_SUFFIX = 1003,
		};

	public:
		class Code;
		class Position;

		/*\
		 - Script::Code - where symbolic data lives in peaceful bliss
		\*/
		class Code
		{
			friend class Script::Position;

			CodeList				m_Code;
			long					m_NumSymbols = 0;
			long					m_NumLines = 0;

			inline CodeList	& GetLines()		{ return m_Code; }
			inline CodeList & operator *()		{ return GetLines(); }
			inline CodeList * operator ->()		{ return &**this; }

		public:
			Code();
			Code(const CodeList & code);

			inline const CodeList	&	GetLines() const	{ return m_Code; }
			inline long 				NumSymbols() const	{ return m_NumSymbols; }
			inline long 				NumLines() const	{ return m_NumLines; }

			inline const CodeList & operator *() const		{ return GetLines(); }
			inline const CodeList * operator ->() const		{ return &**this; }

			inline void AddLine(const CodeLine & code) {
				if (!code.Symbols().empty())
				{
					m_Code.emplace_back(++m_NumLines, code, nullptr);
					m_NumSymbols += code.Symbols().size();
				}
				else ++m_NumLines;
			}
			inline void Clear() {
				m_Code.clear();
				m_NumLines = 0;
				m_NumSymbols = 0;
			}

			// Erase the code from Position A to (and including) Position B
			// Updates all positions automatically and returns the reference to Position A
			Position & Erase(Position &, Position &);
		};

		/*\
		 - Script::Position(tm) - iterating through all that matters since '14
		\*/
		class Position
		{
			friend class Code;

			Code					*	m_pCode;
			//ScriptLine				*	m_pScriptLine = nullptr;
			CodeList::iterator			m_LineIt;		// (x)
			CodeLine::iterator			m_CodeIt;		// (y)

			inline CodeList::iterator	GetLineIt()		{ return m_LineIt; }
			inline CodeLine::iterator	GetSymbolIt()	{ return m_CodeIt; }
			inline Code				&	GetCode()		{ ASSERT(m_pCode); return *m_pCode; }
			inline const Code		&	GetCode() const	{ ASSERT(m_pCode); return *m_pCode; }

			void GetCodeLine();
			bool NextLine();

		public:
			Position();
			Position(Code & code);
			//Position(const Position & pos);

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
			Script::Position & Delete();

			/*\
			 - Returns true if this position is at the end of the symbol list
			\*/
			inline bool IsEnd() const {
				return m_LineIt == GetCode()->end() || m_CodeIt == m_LineIt->GetCode().Symbols().end();
			}

			// Get the current line of this position
			inline ScriptLine & GetLine()					{ return *m_LineIt; }
			inline const ScriptLine & GetLine()	const		{ return *m_LineIt; }

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

			// 
			inline Position & operator=(const Position & pos) {
				m_pCode = pos.m_pCode;
				m_CodeIt = pos.m_CodeIt;
				m_LineIt = pos.m_LineIt;
				return *this;
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
		};
		
	private:
		Code								m_Code;
		//StringList							m_Errors;
		StringList							m_Warnings;
		//MacroMap							m_Macros;
		//std::map<std::string, Macro>		m_Macros;
		//std::map<std::map, Variable>		m_Variables;

		int									m_nCommentDepth;

		std::vector<bool>					m_PreprocessorHistory;


		// Initialise script for parsing with current code
		void Init();

		void Error(int code, const std::string & msg);

	public:
		Script()
		{ }

		// Construct script parser with code from memory
		Script(const CodeList & code);

		~Script()
		{ }

		// Load file into code lines
		void LoadFile(const std::string & path);
		
		// Include file in specific code line
		//void IncludeFile(const std::string & path, CodeList::iterator line);

		inline bool OK() const { return true; }
		inline size_t GetNumLines() const { return m_Code.NumLines(); }
		inline bool InComment() const { return m_nCommentDepth > 0; }

		inline Code & GetCode() { return m_Code; }

		//inline CMacros & Macros() { return m_Macros; };
		//inline const CMacros & Macros() const { return m_Macros; };

		// For preprocessor eyes only

		void PushSourceControl(bool b) { m_PreprocessorHistory.push_back(b); }
		void PopSourceControl() {
			ASSERT(!m_PreprocessorHistory.empty());
			m_PreprocessorHistory.pop_back();
		}
		void InvertSourceControl() { m_PreprocessorHistory.back() = !m_PreprocessorHistory.back(); }
		bool GetSourceControl() const {
			ASSERT(!m_PreprocessorHistory.empty()); // if this activates, you popped too much!
			return m_PreprocessorHistory.back();
		}
		//long long EvaluateExpression(const std::string & expr, int depth = 0);
	};
}