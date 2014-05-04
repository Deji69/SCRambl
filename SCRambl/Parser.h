/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
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

	int ExprToChar(const std::string& str, size_t * pos = nullptr);

	template<class T>
	T ExprToInt(const std::string& str, size_t * pos = nullptr, int base = 0);

	template<> inline long long ExprToInt<long long>(const std::string& str, size_t * pos, int base)
	{
		return std::stoll(widen(str), pos, base);
	}

	template<> inline unsigned long long ExprToInt<unsigned long long>(const std::string& str, size_t * pos, int base)
	{
		return std::stoull(widen(str), pos, base);
	}

	template<class T>
	T ExprToFlt(const std::string& str, size_t * pos = nullptr);
	
	inline std::string GetIdentifier(std::string & src)
	{
		std::string str(std::find_if(src.begin(), src.end(), std::not1(std::function<int(int)>(IsSpace))), src.end());
		return std::string(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::function<int(int)>(IsIdentifier))));
	}
}