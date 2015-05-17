/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once

#include <assert.h>
#include <algorithm>
#include <functional>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
	#include <Windows.h>
#endif

namespace SCRambl
{
	template<class T>
	inline T * szncpy(T *out, const char *src, size_t size);
}

#define ASSERT(expr) do{assert(expr);}while(0)
#ifdef _WIN32
	#define BREAK() do{__debugbreak();}while(0)
#else
	#define BREAK() do{}while(0)
#endif

#include "utils/hash.h"
#include "utils/utf8.h"
#include "utils/ansi.h"
#include "utils/key.h"
#include "utils/map.h"
#include "utils/function_traits.h"
#include "utils/xml.h"

namespace SCRambl
{
	union VersionStruct
	{
		struct {
			uint32_t Major : 8;
			uint32_t Minor : 8;
			uint32_t Beta : 8;
			uint32_t Alpha : 8;
		};
		uint32_t VersionLong;

		VersionStruct() : VersionLong(0) { }
		VersionStruct(uint32_t v) : VersionLong(v) { }
		VersionStruct(uint8_t major, uint8_t minor, uint8_t beta = 0, uint8_t alpha = 0) : Alpha(alpha), Beta(beta), Minor(minor), Major(major) { }
		inline bool operator==(const VersionStruct& v) const { return VersionLong == v.VersionLong; }
		inline bool operator!=(const VersionStruct& v) const { return !(*this == v); }
		inline bool operator>(const VersionStruct& v) const { return Major > v.Major || Minor > v.Minor || Beta > v.Beta || Alpha > v.Alpha; }
		inline bool operator<(const VersionStruct& v) const { return Major < v.Major || Minor < v.Minor || Beta < v.Beta || Alpha < v.Alpha; }
		inline bool operator>=(const VersionStruct& v) const { return !(*this < v); }
		inline bool operator<=(const VersionStruct& v) const { return !(*this > v); }

		// Will get the shortest possible version string (omitting unnecessary .0) e.g. 1.0.0.0 == 1, 1.0.2.0 = 1.0.2, 0.0.0.0 = 0
		std::string GetString() const;
	};

	extern const VersionStruct SCRamblVersion;

	// for merging vectors, without duplicates, through remove_copy_if, etc.
	template<typename T>
	struct VectorContainPred {
		const std::vector<T>& vec;
		VectorContainPred(const std::vector<T>& v) : vec(v) { }
		inline bool operator()(T v) {
			return vec.end() != std::find(vec.begin(), vec.end(), v);
		}
	};

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

	template<typename T>
	inline size_t CountBitOccupation(T N) {
		auto v = (size_t)std::log2(N);
		return v ? v : 1;
	}

	template<typename T>
	inline size_t CountByteOccupation(T v) {
		size_t n = CountBitOccupation<unsigned long>(v);
		return (n / 8) + (n % 8 ? 1 : 0);
	}

	// Returns the pos of the 'ext' part of "any.path/file.ext", or npos if no extension is found
	inline size_t GetFilePathExtensionPos(const std::string& path) {
		for (size_t i = path.length(); i; --i) {
			if (path[i] == '.') {
				return i + 1;
			}
			if (path[i] == '\\' || path[i] == '//')
				break;
		}
		return std::string::npos;
	}
	inline std::string GetFilePathExtension(const std::string& path) {
		return path.substr(GetFilePathExtensionPos(path));
	}

#ifdef _WIN32
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
#endif
}