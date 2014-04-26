#include "stdafx.h"
#include "Parser.h"

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
	int IsSpace(int c)
	{
		return std::isspace(c);
	}
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
}