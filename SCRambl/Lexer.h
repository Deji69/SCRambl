#pragma once
#include <string>
#include "utils.h"

namespace SCRambl
{
	enum eLexToken
	{
		LEX_EOF = -1,
		LEX_EOL = LEX_EOF,		// no more
		LEX_NULL = 0,			// nada
		LEX_PUNCTUATOR,			// stuff that separates the other stuff, but isn't a whitespace
		LEX_NUMBER,				// duhhh
		LEX_LABEL,				// identifiers before :
		LEX_STRING,				// "all ovv thisssss"
		LEX_OPERATOR,			// subset of punctuators
		LEX_IDENTIFIER,			// the nouns of programming languages
		LEX_DIRECTIVE,			// #anythinglikethis
		LEX_SUBSCRIPT,			// not_this[but_this]
		LEX_UNKNOWN,				// wtf?
	};

	class CSCRLexer
	{
		friend class CTokenInfo;
		static inline bool			IsIdentifier(int ch)
		{
			return IsAlphanumeric(ch) || ch == '_' || ch == '.';		// R* likes .'s too
		}
		static inline bool			IsNumber(int ch)
		{
			return IsDecimal(ch);
		}
		static inline bool			IsPunctuator(int ch)
		{
			switch (ch)
			{
			case ',': case ':': case '&': case '\\':
			case '{': case '}':
			case '(': case ')':
			case '[': case ']':
				return true;
			}
			return IsOperator(ch);
		}
		static inline bool			IsOperator(int ch)
		{
			switch (ch)
			{
			case '<': case '>':
			case '#': case '=':
			case '+': case '-': case '*': case '/':
			case '@':
				// (new)
			case '^': case '~': case '%':
				return true;
			}
			return false;
		}
		static inline bool			SkipWhiteSpace(const char *&str)
		{
			auto old = str;
			while (IsSpace(*str)) ++str;
			return old != str;
		}

	public:
	};

	class CTokenInfo
	{
		eLexToken					m_eType;
		const char				*	m_szTokenBegin;
		const char				*	m_szTokenEnd;

		inline void			Nullify()
		{
			m_eType = LEX_NULL;
			m_szTokenBegin = nullptr;
			m_szTokenEnd = nullptr;
		}

	public:
		CTokenInfo()
		{
			Nullify();
		}
		CTokenInfo(const char * szCode)
		{
			Generate(szCode);
		}
		inline eLexToken		GetType() const							{ return m_eType; }
		inline const char *	GetBegin() const							{ return m_szTokenBegin; }
		inline const char *	GetEnd()	 const							{ return m_szTokenEnd; }
		inline std::string	GetString()	const						{ return std::string(m_szTokenBegin, m_szTokenEnd); }
		inline size_t		GetLength() const						{ return m_szTokenEnd - m_szTokenBegin; }

		bool					Generate(const char * szCode)
		{
			m_eType = LEX_NULL;

			while (*szCode)
			{
				// skip spaces
				if (CSCRLexer::SkipWhiteSpace(szCode))
					continue;
				// check for directive
				if (*szCode == '#')
				{
					CSCRLexer::SkipWhiteSpace(++szCode);
					m_szTokenEnd = m_szTokenBegin = szCode;
					do ++m_szTokenEnd;
					while (*m_szTokenEnd && !IsSpace(*m_szTokenEnd) && !CSCRLexer::IsPunctuator(*m_szTokenEnd));
					m_eType = LEX_DIRECTIVE;
				}
				// check for string
				else if (*szCode == '"')
				{
					m_szTokenEnd = m_szTokenBegin = ++szCode;
					bool bEscape = false;
					while (*m_szTokenEnd)
					{
						if (*m_szTokenEnd == '"')
						{
							if (!bEscape) break;
							else bEscape = false;
						}
						else if (*m_szTokenEnd == '\\')
							bEscape = bEscape != true;
						else bEscape = false;
						++m_szTokenEnd;
					}
					m_eType = LEX_STRING;
				}
				// check for subscripts
				else if (*szCode == '[')
				{
					int nDepth = 1;
					m_szTokenEnd = m_szTokenBegin = ++szCode;
					for (; *m_szTokenEnd; ++m_szTokenEnd)
					{
						if (*m_szTokenEnd == ']')
						{
							if (!--nDepth) break;
						}
						else if (*m_szTokenEnd == '[') ++nDepth;
					}
					m_eType = LEX_SUBSCRIPT;
				}
				// check for value
				else if (CSCRLexer::IsNumber(*szCode) || ((*szCode == '+' || *szCode == '-') && CSCRLexer::IsNumber(szCode[1])))
				{
					m_szTokenEnd = m_szTokenBegin = szCode;
					do ++m_szTokenEnd;
					while (*m_szTokenEnd && !IsSpace(*m_szTokenEnd) && !CSCRLexer::IsPunctuator(*m_szTokenEnd));
					m_eType = LEX_NUMBER;
				}
				// check for operator
				else if (CSCRLexer::IsOperator(*szCode))
				{
					m_szTokenEnd = m_szTokenBegin = szCode;
					do ++m_szTokenEnd;
					while (*m_szTokenEnd && CSCRLexer::IsOperator(*m_szTokenEnd));
					m_eType = LEX_OPERATOR;
				}
				// check for identifier / label
				else if (CSCRLexer::IsIdentifier(*szCode))
				{
					m_szTokenEnd = m_szTokenBegin = szCode;
					do ++m_szTokenEnd;
					while (*m_szTokenEnd && !IsSpace(*m_szTokenEnd) && !CSCRLexer::IsPunctuator(*m_szTokenEnd));
					m_eType = *m_szTokenEnd == ':' ? LEX_LABEL : LEX_IDENTIFIER;
				}
				// return any other punctuator
				else if (CSCRLexer::IsPunctuator(*szCode))
				{
					m_szTokenEnd = m_szTokenBegin = szCode;
					++m_szTokenEnd;
					m_eType = LEX_PUNCTUATOR;
				}
				else
				{
					m_szTokenEnd = m_szTokenBegin = szCode;
					do ++m_szTokenEnd;
					while (*m_szTokenEnd && !IsSpace(*m_szTokenEnd) && !CSCRLexer::IsIdentifier(*m_szTokenEnd) && !CSCRLexer::IsPunctuator(*m_szTokenEnd));
					m_eType = LEX_UNKNOWN;
				}
				break;
			}
			if (m_eType != LEX_NULL) return true;
			m_szTokenBegin = nullptr;
			m_szTokenEnd = nullptr;
			return true;
		}
	};
}