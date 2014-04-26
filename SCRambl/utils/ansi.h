#pragma once
#include <cctype>

namespace SCRambl
{
	// For checking characters
	inline bool IsAlphabetic(char c)		{ return isalpha((int)c) != 0; }
	inline bool IsAlphanumeric(char c)	{ return isalnum((int)c) != 0; }
	inline bool IsDecimal(char c)		{ return isdigit((int)c) != 0; }
	inline bool IsHex(char c)			{ return isxdigit((int)c) != 0; }
	inline bool IsSpace(char c)			{ return isspace((int)c) != 0; }
	inline bool IsPrint(char c)			{ return isprint((int)c) != 0; }
	inline bool IsUpper(char c)			{ return isupper((int)c) != 0; }
	inline bool IsLower(char c)			{ return islower((int)c) != 0; }
}