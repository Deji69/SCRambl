/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once

#include <string>
#include <vector>
#include <assert.h>
#include <map>
#include <unordered_map>
#include "utils.h"
#include "Identifiers.h"

namespace SCRambl
{
	class Symbol
	{
	public:
		enum Type : char {
			whitespace,
			eol,
			identifier,
			delimiter,
			number,
			punctuator,
			separator,
			unknown
		};

	private:
		const Type		m_Type;
		const char		m_Character;

		static Type GetCharSymbolType(char character)
		{
			static const std::vector<const Type> character_set = {
							// Control chars
				/* 00 */	{ /*NUL*/ eol },
				/* 01-08 */	{ /*SOH*/ unknown }, { /*STX*/ unknown }, { /*ETX*/ unknown }, { /*EOT*/ unknown },
							{ /*ENQ*/ unknown }, { /*ACK*/ unknown }, { /*BEL*/ unknown }, { /*BS */ unknown },
				/* 09-0F */	{ /*HT */ whitespace }, { /*LF */ whitespace }, { /*VT */ whitespace }, { /*FF */ whitespace },
							{ /*CR */ whitespace }, { /*SO */ unknown }, { /*SI */ unknown },
				/* 10-1F */	{ /*DLE*/ unknown }, { /*DC1*/ unknown }, { /*DC2*/ unknown }, { /*DC3*/ unknown },
							{ /*DC4*/ unknown }, { /*NAK*/ unknown }, { /*SYN*/ unknown }, { /*ETB*/ unknown },
							{ /*CAN*/ unknown }, { /*EM */ unknown }, { /*SUB*/ unknown }, { /*ESC*/ unknown },
							{ /*FS */ unknown }, { /*GS */ unknown }, { /*RS */ unknown }, { /*US */ unknown },

							// Printables
				/* 20-2F */	{ /*   */ whitespace }, { /* ! */ punctuator }, { /* " */ delimiter }, { /* # */ punctuator },
							{ /* $ */ punctuator }, { /* % */ punctuator }, { /* & */ punctuator }, { /* ' */ delimiter },
							{ /* ( */ punctuator }, { /* ) */ punctuator }, { /* * */ punctuator }, { /* + */ unknown },
							{ /* , */ separator }, { /* - */ punctuator }, { /* . */ separator }, { /* / */ punctuator },
				/* 30-3F */	{ /* 0 */ number }, { /* 1 */ number }, { /* 2 */ number }, { /* 3 */ number },
							{ /* 4 */ number }, { /* 5 */ number }, { /* 6 */ number }, { /* 7 */ number },
							{ /* 8 */ number }, { /* 9 */ number }, { /* : */ punctuator }, { /* ; */ punctuator },
							{ /* < */ punctuator }, { /* = */ punctuator }, { /* > */ punctuator }, { /* ? */ punctuator },
				/* 40-5F */	{ /* @ */ punctuator }, { /* A */ identifier }, { /* B */ identifier }, { /* C */ identifier },
							{ /* D */ identifier }, { /* E */ identifier }, { /* F */ identifier }, { /* G */ identifier },
							{ /* H */ identifier }, { /* I */ identifier }, { /* J */ identifier }, { /* K */ identifier },
							{ /* L */ identifier }, { /* M */ identifier }, { /* N */ identifier }, { /* O */ identifier },
							{ /* P */ identifier }, { /* Q */ identifier }, { /* R */ identifier }, { /* S */ identifier },
							{ /* T */ identifier }, { /* U */ identifier }, { /* V */ identifier }, { /* W */ identifier },
							{ /* X */ identifier }, { /* Y */ identifier }, { /* Z */ identifier }, { /* [ */ delimiter },
							{ /* \ */ punctuator }, { /* ] */ delimiter }, { /* ^ */ punctuator }, { /* _ */ identifier },
				/* 60-7F */ { /* ` */ unknown }, { /* a */ identifier }, { /* b */ identifier }, { /* c */ identifier },
							{ /* d */ identifier }, { /* e */ identifier }, { /* f */ identifier }, { /* g */ identifier },
							{ /* h */ identifier }, { /* i */ identifier }, { /* j */ identifier }, { /* k */ identifier },
							{ /* l */ identifier }, { /* m */ identifier }, { /* n */ identifier }, { /* o */ identifier },
							{ /* p */ identifier }, { /* q */ identifier }, { /* r */ identifier }, { /* s */ identifier },
							{ /* t */ identifier }, { /* u */ identifier }, { /* v */ identifier }, { /* w */ identifier },
							{ /* x */ identifier }, { /* y */ identifier }, { /* z */ identifier }, { /* { */ delimiter },
							{ /* | */ punctuator }, { /* } */ delimiter }, { /* ~ */ punctuator }, { /* */ unknown },

							// Extended...?      
			};
			int8_t ch = character;
			ASSERT(character >= 0);
			ASSERT(character_set.size() == 0x80);
			return character_set[ch];
		}

		static char GetGenericTypeChar(Type generic_type)
		{
			switch (generic_type)
			{
			case whitespace: return ' ';
			case eol: return '\0';
			}
			BREAK();
			return ' ';
		}

	public:
		Symbol(char character) : m_Character(character), m_Type(GetCharSymbolType(character))
		{}
		Symbol(Type generic_type) : m_Character(GetGenericTypeChar(generic_type)), m_Type(generic_type)
		{}

		inline char GetChar() const			{ return m_Character; }
		inline Type GetType() const			{ return m_Type; }
		inline operator char() const		{ return GetChar(); }
	};

	class CodeLine
	{
		std::vector<Symbol>			m_Symbols;

	public:
		inline std::vector<Symbol> & Symbols() { return m_Symbols; }
		inline const std::vector<Symbol> & Symbols() const { return m_Symbols; }
	};

#if 0
	class CodeChar
	{
		friend class CodeLine;

		enum Type : char {
			whitespace,
			eol,
			identifier,
			delimiter,
			number,
			string,
			punctuator,
		} m_Type;
		char m_Character;

		CodeChar(Type type, char character) : m_Type(type), m_Character(character)
		{}

	public:
		CodeChar(char character) : m_Character(character), m_Type(GetCharType(character))
		{
		}

		static Type GetCharType(char character)
		{
			switch (character)
			{
			default:
				if (IsSpace(character)) return whitespace;
				if (IsDecimal(character)) return number;
				if (!IsIdentifier(character)) break;
			case '.':
			case '_':
				return identifier;
			case '\0':
				return eol;
			case '"': case '\'':
			case '!': case '#': case '%': case '&': case '(': case ')':
			case '*': case '+': case ',': case '-': case '/': case ':':
			case ';': case '<': case '=': case '>': case '?': case '[':
			case '\\': case ']': case '^': case '{': case '|': case '}':
			case '~':
				return punctuator;
			}
		}

	public:
		/*bool operator==(char c)  const
		{
			return c == m_Character;
		}*/
		inline operator char() const
		{
			return m_Character;
		}
	};

	typedef std::vector<CodeChar> Code;

	class CodeTranslator
	{
		enum State { before, inside, after } m_State;

		std::map<char, std::pair<State, char>>		m_BeforeMap;
		std::map<char, std::pair<State, char>>		m_DuringMap;
		std::map<char, std::pair<State, char>>		m_AfterMap;
	};

	class CodeLine
	{
		Code						m_Code;

		/*static char GetTrigraphChar(char c)
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
			return '\0';
		}

		inline void AddCharacter(char c)
		{
			auto & last = m_Code.empty() ? '\0' : m_Code.back();
			if (last == '0')
			{
			}
			m_Code.emplace_back(c);
		}
		inline bool AddTrigraph(char c)
		{
			if (char n = GetTrigraphChar(c))
			{
				AddCharacter(n);
				return true;
			}
			return false;
		}*/

	public:
		CodeLine(const std::string & str)
		{}
		/*CodeLine(const char * line)
		{
			// Do easy phases 1 & 2...

			// Scan all chars, including the null terminator which will indicate the eol unless escaped
			for (const char * pc = line; pc[-1] != '\0'; ++pc)
			{
				switch (pc[0])
				{
				case '?':
					if (pc[1] == '?' && AddTrigraph(pc[3]))
					{
						pc += 2;
						continue;
					}
					break;
				case '\\':
					switch (pc[1])
					{
						// don't add eol char if a backslash ends the line
					case '\0':
						pc += 2;
						continue;
					}
					break;
				}

				AddCharacter(pc[0]);
			}
		}

		inline operator std::vector<CodeChar> &()					{ return m_Code; }
		inline operator const std::vector<CodeChar> &() const		{ return m_Code; }*/
	};
#endif
}