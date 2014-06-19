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

namespace SCRambl
{
	class Grapheme
	{
	public:
		enum ID : char {
			not, hash, dollar, percent, and, left_parenthesis, right_parenthesis, asterisk,
			plus, minus, slash, colon, semicolon, less, equals, more, condition,
			at, backslash, caret, pipe, tilde,
			max_type
		};

	private:
		ID				m_ID = max_type;

	public:
		Grapheme() : m_ID(max_type)
		{ }
		Grapheme(ID id) : m_ID(id)
		{ }

		inline operator ID() const	 { return m_ID; }
	};

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
		Type		m_Type;				// type of symbol
		//int		m_Column;			// column on source line		// TODO: implement later - too lazy now
		char		m_Character;		// symbol character
		Grapheme	m_Grapheme;			// grapheme

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

		void GetCharSymbolType(char character)
		{
			struct charsetpair { Type first; Grapheme second; };
			static const std::vector <charsetpair> character_set = {
				// Control chars
				/* 00 */	{ /*NUL*/ eol },
				/* 01-08 */	{ /*SOH*/ unknown },	{ /*STX*/ unknown },	{ /*ETX*/ unknown },	{ /*EOT*/ unknown },
							{ /*ENQ*/ unknown },	{ /*ACK*/ unknown },	{ /*BEL*/ unknown },	{ /*BS */ unknown },
				/* 09-0F */	{ /*HT */ whitespace }, { /*LF */ whitespace }, { /*VT */ whitespace }, { /*FF */ whitespace },
							{ /*CR */ whitespace }, { /*SO */ unknown },	{ /*SI */ unknown },
				/* 10-1F */	{ /*DLE*/ unknown },	{ /*DC1*/ unknown },	{ /*DC2*/ unknown },	{ /*DC3*/ unknown },
							{ /*DC4*/ unknown },	{ /*NAK*/ unknown },	{ /*SYN*/ unknown },	{ /*ETB*/ unknown },
							{ /*CAN*/ unknown },	{ /*EM */ unknown },	{ /*SUB*/ unknown },	{ /*ESC*/ unknown },
							{ /*FS */ unknown },	{ /*GS */ unknown },	{ /*RS */ unknown },	{ /*US */ unknown },
				// Printables
				/* 20-2F */	{ /*   */ whitespace }, { /* ! */ punctuator, Grapheme::not },
							{ /* " */ delimiter },	{ /* # */ punctuator, Grapheme::hash },
							{ /* $ */ punctuator, Grapheme::dollar },
							{ /* % */ punctuator, Grapheme::percent },
							{ /* & */ punctuator, Grapheme::and },
							{ /* ' */ delimiter },
							{ /* ( */ punctuator, Grapheme::left_parenthesis },
							{ /* ) */ punctuator, Grapheme::right_parenthesis },
							{ /* * */ punctuator, Grapheme::asterisk },
							{ /* + */ punctuator, Grapheme::plus },
							{ /* , */ separator },
							{ /* - */ punctuator, Grapheme::minus },
							{ /* . */ identifier },
							{ /* / */ punctuator, Grapheme::slash },
				/* 30-3F */	{ /* 0 */ number },		{ /* 1 */ number },		{ /* 2 */ number },		{ /* 3 */ number },
							{ /* 4 */ number },		{ /* 5 */ number },		{ /* 6 */ number },		{ /* 7 */ number },
							{ /* 8 */ number },		{ /* 9 */ number },
							{ /* : */ punctuator, Grapheme::colon },
							{ /* ; */ punctuator, Grapheme::semicolon },
							{ /* < */ punctuator, Grapheme::less },
							{ /* = */ punctuator, Grapheme::equals },
							{ /* > */ punctuator, Grapheme::more },
							{ /* ? */ punctuator, Grapheme::condition },
				/* 40-5F */	{ /* @ */ punctuator, Grapheme::at },
							{ /* A */ identifier }, { /* B */ identifier }, { /* C */ identifier },
							{ /* D */ identifier }, { /* E */ identifier }, { /* F */ identifier }, { /* G */ identifier },
							{ /* H */ identifier }, { /* I */ identifier }, { /* J */ identifier }, { /* K */ identifier },
							{ /* L */ identifier }, { /* M */ identifier }, { /* N */ identifier }, { /* O */ identifier },
							{ /* P */ identifier }, { /* Q */ identifier }, { /* R */ identifier }, { /* S */ identifier },
							{ /* T */ identifier }, { /* U */ identifier }, { /* V */ identifier }, { /* W */ identifier },
							{ /* X */ identifier }, { /* Y */ identifier }, { /* Z */ identifier }, { /* [ */ delimiter },
							{ /* \ */ punctuator, Grapheme::backslash },
							{ /* ] */ delimiter },
							{ /* ^ */ punctuator, Grapheme::caret },
							{ /* _ */ identifier },
				/* 60-7F */ { /* ` */ unknown },	{ /* a */ identifier },	{ /* b */ identifier }, { /* c */ identifier },
							{ /* d */ identifier }, { /* e */ identifier }, { /* f */ identifier }, { /* g */ identifier },
							{ /* h */ identifier }, { /* i */ identifier }, { /* j */ identifier }, { /* k */ identifier },
							{ /* l */ identifier }, { /* m */ identifier }, { /* n */ identifier }, { /* o */ identifier },
							{ /* p */ identifier }, { /* q */ identifier }, { /* r */ identifier }, { /* s */ identifier },
							{ /* t */ identifier }, { /* u */ identifier }, { /* v */ identifier }, { /* w */ identifier },
							{ /* x */ identifier }, { /* y */ identifier }, { /* z */ identifier }, { /* { */ delimiter },
							{ /* | */ punctuator, Grapheme::pipe },
							{ /* } */ delimiter },
							{ /* ~ */ punctuator, Grapheme::tilde },
							{ /* */ unknown },
				// Extended...
			};
			int8_t ch = character;
			ASSERT(character >= 0 && character <= 127);
			ASSERT(character_set.size() == 0x80);
			m_Type = character_set[ch].first;
			m_Grapheme = character_set[ch].second;
		}

	public:
		Symbol(char character):
			m_Character(character)
		{
			GetCharSymbolType(character);
		}
		Symbol(Type generic_type):
			m_Type(generic_type),
			m_Character(GetGenericTypeChar(generic_type))
		{}

		inline char GetChar() const				{ return m_Character; }
		inline Type GetType() const				{ return m_Type; }
		inline Grapheme GetGrapheme() const		{ return m_Grapheme; }
		inline operator char() const			{ return GetChar(); }
		inline bool HasGrapheme() const			{ return m_Grapheme != Grapheme::max_type; }
		inline bool IsIgnorable() const			{ return m_Type == whitespace; }
		inline bool IsSeparating() const {
			switch (m_Type)
			{
			case whitespace:
			case separator:
			case delimiter:
			case punctuator:
			case eol:
				return true;
			}
			return false;
		}
	};

	class CodeLine
	{
	public:
		typedef std::vector<Symbol> vector;
		typedef vector::iterator iterator;
		typedef vector::const_iterator const_iterator;

	private:
		std::vector<Symbol>			m_Symbols;

	public:
		CodeLine() = default;
		CodeLine(const vector symbols) : m_Symbols(symbols)
		{}

		inline vector & Symbols() { return m_Symbols; }
		inline const vector & Symbols() const { return m_Symbols; }
		inline std::string String() const {
			std::string r;
			for (char c : m_Symbols) r += c;
			return r;
		}

		inline CodeLine & operator=(const std::string & str) {
			m_Symbols.clear();
			if (!str.empty())
			{
				for (auto c : str) {
					m_Symbols.emplace_back(c);
				}
			}
			return *this;
		}
	};
}