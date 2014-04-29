#include "stdafx.h"
#include "Parser.h"
#include "Exception.h"

#include <cctype>

namespace SCRambl
{
	int IsDirectivePrefix(int c)
	{
		return c == '#';
	}
	int IsIdentifierStart(int c)
	{
		return std::isalpha(c) || c == '_';
	}
	int IsIdentifier(int c)
	{
		return std::isalnum(c) || c == '_' || c == '.';
	}
	/*int IsSpace(int c)
	{
		return std::isspace(c);
	}*/
	int BothAreSpaces(int a, int b)
	{
		return IsSpace(a) && IsSpace(b);
	}
	int IsSeparator(int c)
	{
		if (IsSpace(c)) return true;
		switch (c)
		{
		case ',':
		case ':':
		case '&':
		case '\\':
		case '{': case '}':
		case '(': case ')':
		case '[': case ']':
		case '<': case '>':
		case '#':
		case '=':
		case '+':
		case '-':
		case '*':
		case '/':
		case '@':
			return true;
		}
		return false;
	}
	int BothAreSeparators(int a, int b)
	{
		return IsSeparator(a) && IsSeparator(b);
	}

	// if the 2nd character is a useless white-space because the last character is already a separator
	int IsUselessSeparator(int a, int b)
	{
		return IsSeparator(a) && IsSpace(b);
	}


	// Get literal char representation from string
	int ExprToChar(const std::string& str, size_t * pos)
	{
		int c;
		size_t n;
		for (n = 0; n < str.length(); ++n)
		{
			if (!IsPrint(str[n])) throw(std::invalid_argument("Non-printing character used in literal string"));
			if (str[n] == '\\')
			{
				++n;
				switch (str[n])
				{
				case 'a':
					c = '\a';
					break;
				case 'b':
					c = '\b';
					break;
				case 'f':
					c = '\f';
					break;
				case 'n':
					c = '\n';
					break;
				case 'r':
					c = '\r';
					break;
				case 't':
					c = '\t';
					break;
				case 'v':
					c = '\v';
					break;
				case 'x':
				{
					size_t size;
					++n;
					try
					{
						c = (int)ExprToInt<long long>(str.substr(n), &size);
						n += size;
					}
					catch (...)
					{
						throw;
					}
					
					// if \ directly followed the hex sequence, skip it (so "MAdam" can be written "\x4D\Adam" with "Ada" not part of the hex)
					if (str[n] == '\\') ++n;
					break;
				}
				case '\\':
					c = '\\';
					break;
				case '?':
					c = '?';
					break;
				case '\'':
					c = '\'';
					break;
				case '"':
					c = '\"';
					break;
				case '0':
					c = '\0';

					// TODO: octal? (the utterly useless alternative to everything?)
					break;
				default:
					c = str[n];
					break;
				}
				break;
			}
			else
			{
				c = str[n];
				break;
			}
		}
		if (pos) *pos = n;
	}

	// looonnggg
	template<> long long ExprToInt<long long>(const std::string& str, size_t * pos, int base)
	{
		return std::stoll(widen(str), pos, base);
	}

	template<> unsigned long long ExprToInt<unsigned long long>(const std::string& str, size_t * pos, int base)
	{
		return std::stoull(widen(str), pos, base);
	}

	template<> double ExprToFlt<double>(const std::string & str, size_t * pos)
	{
		return std::stod(widen(str), pos);
	}
	
	template<> float ExprToFlt<float>(const std::string & str, size_t * pos)
	{
		return std::stof(widen(str), pos);
	}
}