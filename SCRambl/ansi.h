#ifndef UTIL_ANSI_H
#define UTIL_ANSI_H
#include <cctype>

// For checking characters
inline int IsAlphabetic(int c)		{ return std::isalpha(c) != 0; }
inline int IsAlphanumeric(int c)		{ return std::isalnum(c) != 0; }
inline int IsDecimal(int c)			{ return std::isdigit(c) != 0; }
inline int IsHex(int c)				{ return std::isxdigit(c) != 0; }
inline int IsSpace(int c)			{ return std::isspace(c) != 0; }
inline int IsPrint(int c)			{ return std::isprint(c) != 0; }
inline int IsUpper(int c)			{ return std::isupper(c) != 0; }
inline int IsLower(int c)			{ return std::islower(c) != 0; }

// For transforming characters
inline int ToUpper(int c)			{ return std::toupper(c); };
inline int ToLower(int c)			{ return std::tolower(c); };

// For checking strings
bool	 IsStringInt(const char *szStr)
{
	if (*szStr == '-') szStr++;
	if (*szStr == '0' && ToLower(szStr[1]) == 'x')
	{
		for (szStr += 2; *szStr; szStr++)
		{
			if (IsSpace(*szStr)) return true;
			else if (!IsHex(*szStr)) return false;
		}
	}
	else if (IsDecimal(*szStr))
	{
		for (szStr++; *szStr; szStr++)
		{
			if (IsSpace(*szStr)) return true;
			else if (!IsDecimal(*szStr)) return false;
		}
	}
	else return false;
	return true;
}

// For transforming strings
char	 * StringToUpper(char *szStr)
{
	char * out = szStr;
	while (*szStr)
	{
		*szStr = ToUpper(*szStr);
		++szStr;
	}
	return out;
}
char * StringToUpper(char *szStr, size_t nLimit)
{
	char * out = szStr;
	for (size_t i = nLimit; i; --i)
	{
		*szStr = ToUpper(*szStr);
		++szStr;
	}
	return out;
}
char		*	StringToLower(char *szStr)
{
	char * out = szStr;
	while (*szStr)
	{
		*szStr = ToLower(*szStr);
		++szStr;
	}
	return out;
}
char		*	StringToLower(char *szStr, size_t nLimit)
{
	char * out = szStr;
	for (size_t i = nLimit; i; --i)
	{
		*szStr = ToLower(*szStr);
		++szStr;
	}
	return out;
}

#endif