#pragma once
#include "utils.h"

namespace SCRambl
{
	int IsDirectivePrefix(int);
	int IsIdentifierStart(int);
	int IsIdentifier(int);
	//int IsSpace(int);
	int BothAreSpaces(int, int);
	int IsSeparator(int);
	int BothAreSeparators(int, int);
	int IsUselessSeparator(int, int);
	
	inline std::string GetIdentifier(std::string & src)
	{
		std::string str(std::find_if(src.begin(), src.end(), std::not1(std::function<int(int)>(IsSpace))), src.end());
		return std::string(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::function<int(int)>(IsIdentifier))));
	}
}