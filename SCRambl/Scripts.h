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
#include "utils.h"
#include "Scripts-code.h"
#include "Tokens.h"
#include "Symbols.h"
#include "TokenInfo.h"
#include "Labels.h"
#include "Types.h"
#include "ScriptObjects.h"

namespace SCRambl
{
	using TokenPtrVec = std::vector<IToken*>;

	namespace Scripts
	{
		using CodeList = std::list<Line>;
		using CodeListRef = CodeList::iterator;
		using Files = std::vector<File>;
		using FileRef = VecRef<File>;

		/*\ Scripts::File - Script files and includes
		\*/
		class File
		{
		public:
			File();
			File(const File&);
			File(File*);
			File(Code*);
			File(std::string);
			File(File*, std::string);
			File(Code*, std::string);
			~File();

			bool IsOpen() const;
			bool IsInclude() const;
			long GetNumLines() const;
			long GetNumIncludes() const;
			std::string GetPath() const;
			Code* GetCode() const;
			void SetCode(Code*);
			bool Open(std::string);
			bool Open(Code*, std::string);
			FileRef IncludeFile(Position&, std::string);

		private:
			void ReadFile(std::ifstream&);

			bool m_FileOpen = false;
			File* m_Parent;
			Code* m_Code;
			Position m_Begin;
			Position m_End;
			Files m_Includes;
			std::string m_Path;
			long m_NumLines = 0;
		};
		
		using Labels = Scope<Label>;
	}

	class Script
	{
	public:
		Script();

		// Open file 
		Scripts::FileRef OpenFile(const std::string&);
		
		// Must be \0 terminated
		void SetCode(const void *);
		
		// Is there a file for the main code?
		bool IsFileOpen() const;
		
		// Returns number of loaded code lines
		long long GetNumLines() const;

		//void LoadFile(const std::string&);
		void ReadFile(std::ifstream&, Scripts::Code&);

		/* Include File
		[in] position
		[in] include file path */
		Scripts::Position Include(Scripts::Position&, const std::string&);

		Tokens::Map GenerateTokenMap();

		// Get the code list
		Scripts::Code& GetCode() { return *m_Code; }
		// Get the labels
		Scripts::Labels& GetLabels() { return m_Labels ; }
		// Get the tokens
		Tokens::Storage& GetTokens() { return m_Tokens; }
		// Get the parse tokens
		Tokens::Storage& GetParseTokens() { return m_ParseTokens; }
		const Tokens::Storage& GetParseTokens() const { return m_ParseTokens; }

	private:
		bool ProcessCodeLine(const std::string&, CodeLine&, bool = false);

		Tokens::Storage m_Tokens;
		Tokens::Storage m_ParseTokens;
		//Parsing::
		Scripts::Files m_Files;
		Scripts::Labels m_Labels;
		Scripts::FileRef m_File;
		Scripts::Code* m_Code;
	};
}