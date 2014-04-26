/****************************************************/
// SCRambl - Scripts.h
// Definitions for SCR script parsing
/****************************************************/
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "utils.h"
#include "FileSystem.h"
#include "Identifiers.h"
#include "Macros.h"

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
		CodeList								m_Code;
		StringList							m_Errors;
		StringList							m_Warnings;
		CMacros								m_Macros;
		//std::map<std::string, Macro>		m_Macros;
		//std::map<std::map, Variable>		m_Variables;

		int									m_nCommentDepth;

		std::vector<bool>					m_PreprocessorHistory;

	public:
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

		// Initialise script for parsing with current code
		void Init();

		// Initialise script for parsing with new code
		void Init(const CodeList &);

		inline bool OK() const { return true; }
		inline size_t GetNumLines() const { return m_Code.size(); }
		inline bool InComment() const { return m_nCommentDepth > 0; }

		inline CMacros & Macros() { return m_Macros; };
		inline const CMacros & Macros() const { return m_Macros; };

		void Preprocess();
		void PreprocessLine(std::string & code);
		void PushSourceControl(bool b) { m_PreprocessorHistory.push_back(b); }
		void PopSourceControl() { m_PreprocessorHistory.pop_back(); }
		void InvertSourceControl() { m_PreprocessorHistory.back() = !m_PreprocessorHistory.back(); }
		bool GetSourceControl() const {
			ASSERT(!m_PreprocessorHistory.empty()); // if this activates, you popped too much!
			return m_PreprocessorHistory.back();
		}
	};
}