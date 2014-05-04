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
#include "Identifiers.h"
#include "Macros.h"
#include "Errors.h"

namespace SCRambl
{
	class ScriptLine
	{
		std::string str;
	public:
		operator std::string() const { return str; }
		friend std::istream & operator>>(std::istream &is, ScriptLine &l)
		{
			do
			{
				if (!std::getline(is, l.str))
				{
					l.str = "";
					break;
				}
				if (!l.str.empty()) trim(l.str);
			}
			while (l.str.empty());
			return is;
		}
	};

	typedef std::vector<std::string> CodeList;
	typedef std::vector<std::string> StringList;

	class Script
	{
		enum eSCR_Error
		{
			SCR_UNKNOWN_ERROR = 1000,			// dummy. no, I mean you're a dummy
			SCR_BAD_DIRECTIVE_CHAR = 1001,
			SCR_BAD_CONSTEXPR_OPERATOR = 1002,
			SCR_BAD_NUMBER_SUFFIX = 1003,
		};
		
		CodeList							m_Code;
		StringList							m_Errors;
		StringList							m_Warnings;
		CMacros								m_Macros;
		//std::map<std::string, Macro>		m_Macros;
		//std::map<std::map, Variable>		m_Variables;

		int									m_nCommentDepth;

		std::vector<bool>					m_PreprocessorHistory;


		// Initialise script for parsing with current code
		void Init();

		// Initialise script for parsing with new code
		void Init(const CodeList &);

		void Error(int code, const std::string & msg);

	public:
		Script()
		{
		}

		// Construct script parser with code from memory
		Script(const CodeList & code)
		{
			Init(code);
		}

		// Construct script parser with code from file
		Script(const std::string & path);

		~Script()
		{
		}

		// Load file into code lines
		void LoadFile(const std::string & path);

		inline bool OK() const { return true; }
		inline size_t GetNumLines() const { return m_Code.size(); }
		inline bool InComment() const { return m_nCommentDepth > 0; }

		inline CMacros & Macros() { return m_Macros; };
		inline const CMacros & Macros() const { return m_Macros; };

		bool Preprocess();
		void PreprocessLine(std::string & code);
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
		long long EvaluateExpression(const std::string & expr, int depth = 0);
	};
}