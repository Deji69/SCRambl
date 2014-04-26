#ifndef UTIL_ANSI_H
#define UTIL_ANSI_H
#include <cctype>

// For checking characters
inline bool IsAlphabetic(int c)		{ return isalpha(c) != 0; }
inline bool IsAlphanumeric(int c)	{ return isalnum(c) != 0; }
inline bool IsDecimal(int c)			{ return isdigit(c) != 0; }
inline bool IsHex(int c)				{ return isxdigit(c) != 0; }
inline bool IsSpace(int c)			{ return isspace(c) != 0; }
inline bool IsPrint(int c)			{ return isprint(c) != 0; }
inline bool IsUpper(int c)			{ return isupper(c) != 0; }
inline bool IsLower(int c)			{ return islower(c) != 0; }

// For transforming characters
inline int ToUpper(int c)			{ return toupper(c); };
inline int ToLower(int c)			{ return tolower(c); };

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