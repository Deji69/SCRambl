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
	class Grapheme {
	public:
		enum ID : char {
			not, hash, dollar, percent, and, left_paren, right_paren, multiply,
			plus, minus, divide, colon, semicolon, less, equals, greater, condition,
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

		inline operator ID() const { return m_ID; }
	};
	class Symbol {
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
		Type		m_Type = unknown;	// type of symbol
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
			static const std::vector<charsetpair> character_set = {
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
							{ /* ( */ delimiter, Grapheme::left_paren },
							{ /* ) */ delimiter, Grapheme::right_paren },
							{ /* * */ punctuator, Grapheme::multiply },
							{ /* + */ punctuator, Grapheme::plus },
							{ /* , */ separator },
							{ /* - */ punctuator, Grapheme::minus },
							{ /* . */ identifier },
							{ /* / */ punctuator, Grapheme::divide },
				/* 30-3F */	{ /* 0 */ number },		{ /* 1 */ number },		{ /* 2 */ number },		{ /* 3 */ number },
							{ /* 4 */ number },		{ /* 5 */ number },		{ /* 6 */ number },		{ /* 7 */ number },
							{ /* 8 */ number },		{ /* 9 */ number },
							{ /* : */ punctuator, Grapheme::colon },
							{ /* ; */ punctuator, Grapheme::semicolon },
							{ /* < */ punctuator, Grapheme::less },
							{ /* = */ punctuator, Grapheme::equals },
							{ /* > */ punctuator, Grapheme::greater },
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
			ASSERT(character_set['('].second == Grapheme::left_paren);
			ASSERT(character_set[')'].second == Grapheme::right_paren);
			m_Type = character_set[ch].first;
			m_Grapheme = character_set[ch].second;
		}

	public:
		//Symbol() = default;
		Symbol(char character) :
			m_Character(character)
		{
			GetCharSymbolType(character);
		}
		Symbol(Type generic_type) :
			m_Type(generic_type),
			m_Character(GetGenericTypeChar(generic_type))
		{ }

		inline char GetChar() const	{ return m_Character; }
		inline Type GetType() const	{ return m_Type; }
		inline Grapheme GetGrapheme() const	{ return m_Grapheme; }
		inline operator char() const { return GetChar(); }
		inline bool HasGrapheme() const	{ return m_Grapheme != Grapheme::max_type; }
		inline bool IsEOL() const { return m_Type == eol; }
		inline bool IsIgnorable() const { return m_Type == whitespace; }
		inline bool IsDelimiter() const	{ return m_Type == delimiter; }
		inline bool IsPunctuator() const { return m_Type == punctuator; }
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
	class Column {
	public:
		typedef std::pair<unsigned long, Symbol> pair;

	private:
		pair m_Column;

	public:
		Column(const pair & col) : m_Column(col)
		{ }
		Column(unsigned long col, Symbol sym) : m_Column(std::make_pair(col, sym))
		{ }

		inline unsigned long Number() const { return m_Column.first; }
		inline Symbol & operator*() { return m_Column.second; }
		inline const Symbol & operator*() const { return m_Column.second; }
		inline Symbol * operator->() { return &m_Column.second; }
		inline const Symbol * operator->() const { return &m_Column.second; }
		inline operator Symbol&() { return m_Column.second; }
		inline operator const Symbol&() const { return m_Column.second; }
		inline operator char() { return m_Column.second; }
	};
	class CodeLine {
	public:
		//typedef std::vector<std::pair<int,Symbol>> vector;
		typedef std::vector<Column> vector;
		typedef vector::iterator iterator;
		typedef vector::const_iterator const_iterator;

	private:
		vector m_Symbols;
		int m_NumCols = 0;

	public:
		CodeLine() = default;
		CodeLine(const vector vec) : m_Symbols(vec)
		{ }
		CodeLine(const std::vector<Symbol> symbols) {
			for (auto c : symbols) {
				if (c == '\t') m_NumCols += 3;
				m_Symbols.emplace_back(++m_NumCols, c);
			}
		}
		CodeLine(std::string symbols) {
			for (auto c : symbols) {
				if (c == '\t') m_NumCols += 3;
				m_Symbols.emplace_back(++m_NumCols, c);
			}
		}
		CodeLine(const char* str) : CodeLine(std::string(str))
		{ }

		inline void Clear() { return m_Symbols.clear(); }
		inline bool Empty() const { return m_Symbols.empty(); }
		inline vector::size_type Size() const { return m_Symbols.size(); }
		inline vector::iterator Begin()	{ return m_Symbols.begin(); }
		inline vector::const_iterator Begin() const { return m_Symbols.cbegin(); }
		inline vector::iterator End() { return m_Symbols.end(); }
		inline vector::const_iterator End() const { return m_Symbols.cend(); }
		inline vector::reference Back()	{ return m_Symbols.back(); }
		inline vector::const_reference Back() const { return m_Symbols.back(); }

		// (stupid STL)
		inline vector::iterator begin() { return Begin(); }
		inline vector::iterator end() { return End(); }
		inline vector::const_iterator begin() const { return Begin(); }
		inline vector::const_iterator end() const { return End(); }

		template<typename... Val>
		inline void Append(Val... v) {
			m_Symbols.emplace_back(++m_NumCols, std::forward<Val>(v)...);
		}
		template<>
		inline void Append(char v) {
			if (v == '\t') m_NumCols += 4 - (m_NumCols % 4);				// a tab takes up to 4 columns - make sure we know
			else ++m_NumCols;
			m_Symbols.emplace_back(m_NumCols, std::forward<char>(v));
		}
		template<>
		inline void Append(Symbol v) {
			if (v.GetChar() == '\t') m_NumCols += 4 - (m_NumCols % 4);		// a tab takes up to 4 columns - make sure we know
			else ++m_NumCols;
			m_Symbols.emplace_back(m_NumCols, std::forward<Symbol>(v));
		}

		inline vector::iterator Insert(vector::const_iterator it, const Symbol& sym) {
			return m_Symbols.emplace(it, ++m_NumCols, sym);
		}
		inline vector::iterator Erase(vector::const_iterator it) {
			return m_Symbols.erase(it);
		}
		inline vector::iterator Erase(vector::const_iterator itA, vector::const_iterator itB) {
			return m_Symbols.erase(itA, itB);
		}

		inline std::string String() const {
			std::string r;
			for (auto p : m_Symbols) r += p;
			return r;
		}

		inline CodeLine& operator=(const std::string& str) {
			m_Symbols.clear();
			if (!str.empty()) {
				for (auto c : str) {
					if (c == '\t') m_NumCols += 3;
					m_Symbols.emplace_back(++m_NumCols, c);
				}
			}
			return *this;
		}
	};
}