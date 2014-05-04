/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once

#include <algorithm>
#include <functional>

namespace SCRambl
{
	template<class T>
	inline T * szncpy(T *out, const char *src, size_t size);
}

#define ASSERT(expr) do{assert(expr);}while(0)
#define BREAK() do{__debugbreak();}while(0)

#include "utils\hash.h"
#include "utils\utf8.h"
#include "utils\ansi.h"
#include "utils\key.h"

namespace SCRambl
{
	class line
	{
		std::string str;
	public:
		operator std::string() const { return str; }
		friend std::istream & operator>>(std::istream &is, line &l)
		{
			std::getline(is, l.str);
			return is;
		}
	};

	// trim from start
	static inline std::string &ltrim(std::string &s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}

	// trim from end
	static inline std::string &rtrim(std::string &s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
	}

	// trim from both ends
	static inline std::string &trim(std::string &s)
	{
		return ltrim(rtrim(s));
	}

	// try to get an identifier


	// strncpy + zero-terminate last character
	template<class T>
	inline T * szncpy(T *out, const char *src, size_t size)
	{
		strncpy(out, src, size);
		out[size - 1] = 0;
		return out;
	}

	inline uint64_t JoinInt32(uint32_t high, uint32_t low)
	{
		ULARGE_INTEGER li;
		li.HighPart = high;
		li.LowPart = low;
		return li.QuadPart;
	}

	inline uint64_t GetTime()
	{
		SYSTEMTIME st;
		FILETIME ft;
		GetSystemTime(&st);
		SystemTimeToFileTime(&st, &ft);
		return JoinInt32(ft.dwHighDateTime, ft.dwLowDateTime);
	}

	inline uint64_t 	GetFileModifiedTime(FILE *hFile)
	{
		BY_HANDLE_FILE_INFORMATION fi;
		GetFileInformationByHandle(hFile, &fi);
		return JoinInt32(fi.ftLastWriteTime.dwHighDateTime, fi.ftLastWriteTime.dwLowDateTime);
	}

	inline uint64_t GetFileModifiedTime(const char *szPath)
	{
		WIN32_FILE_ATTRIBUTE_DATA fi;
		GetFileAttributesEx(widen(szPath).c_str(), GetFileExInfoStandard, &fi);
		return JoinInt32(fi.ftLastWriteTime.dwHighDateTime, fi.ftLastWriteTime.dwLowDateTime);
	}
}