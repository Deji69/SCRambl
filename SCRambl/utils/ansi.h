#pragma once
#include <cctype>

namespace SCRambl
{
	// For checking characters
	inline int IsAlphabetic(int c)		{ return std::isalpha(c) != 0; }
	inline int IsAlphanumeric(int c)		{ return std::isalnum(c) != 0; }
	inline int IsDecimal(int c)			{ return std::isdigit(c) != 0; }
	//inline int IsHex(int c)				{ return std::isxdigit(c) != 0; }
	inline int IsSpace(int c)			{ return std::isspace(c) != 0; }
	inline int IsPrint(int c)			{ return std::isprint(c) != 0; }
	inline int IsUpper(int c)			{ return std::isupper(c) != 0; }
	inline int IsLower(int c)			{ return std::islower(c) != 0; }
}