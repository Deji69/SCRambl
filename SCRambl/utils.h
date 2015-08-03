/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <vector>
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

#if (_MSC_VER <= 1800)
	#define NOTHROW
#else
	#define NOTHROW nothrow
#endif

#include "utils/hash.h"
#include "utils/utf8.h"
#include "utils/ansi.h"
#include "utils/key.h"
#include "utils/map.h"
#include "utils/function_traits.h"
#include "utils/xml.h"

namespace SCRambl {
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
	// index-based vector reference (won't invalidate when vector grows - could point to something unexpected)
	template<typename T, typename TIt = size_t>
	class VecRef {
	public:
		using Vec = std::vector<T>;
		VecRef() = default;
		VecRef(const VecRef&) = default;
		VecRef(std::nullptr_t) : VecRef() { }
		VecRef(Vec* vec) : VecRef(*vec)
		{ }
		VecRef(Vec& vec) : VecRef(vec, vec.size())
		{ }
		template<typename TIndex>
		VecRef(Vec* vec, TIndex index) : VecRef(*vec, index)
		{ }
		template<typename TIndex>
		VecRef(Vec& vec, TIndex index) : m_Vector(&vec), m_Index(index < 0 ? vec.size() + index : index)
		{ }
		virtual ~VecRef() { }

		inline T& operator*() const { return *Ptr(); }
		inline T* operator->() const { return Ptr(); }
		inline operator bool() const { return OK(); }
		inline bool operator==(const T& v) const { return Ptr() == v.Ptr(); }
		inline bool operator!=(const T& v) const { return !(*this == v); }

		inline bool OK() const {
			return m_Vector != nullptr && m_Index < m_Vector->size();
		}
		inline T* Ptr() const {
			return OK() ? &(*m_Vector)[m_Index] : nullptr;
		}
		inline T& Get() const {
			return m_Vector.at(m_Index);
		}
		inline TIt Index() const { return m_Index; }

	private:
		Vec* m_Vector = nullptr;
		TIt m_Index = 0;
	};
	// vector indexes - like iterators, except you know where they are quicker
	template<typename T, typename TKey = size_t, typename TCont = std::vector<T>, typename TIt = TCont::const_iterator>
	class VecIndex {
	public:
		using ValType = T;
		using Vector = TCont;
		using IndexType = TKey;

		VecIndex() = default;
		VecIndex(IndexType index, Vector& vec) : m_Vector(&vec), m_Index(index < 0 ? vec.size() + index : index)
		{ }
		VecIndex(TIt it, Vector& vec) : m_Vector(&vec), m_Index(it != vec.end() ? std::distance<TIt>(std::begin(vec), it) : vec.size())
		{ }
		VecIndex(Vector& vec, IndexType index) : m_Vector(&vec), m_Index(index < 0 ? vec.size() + index : index)
		{ }
		VecIndex(Vector& vec, TIt it) : m_Vector(&vec), m_Index(it != vec.end() ? std::distance<TIt>(std::begin(vec), it) : vec.size())
		{ }
		virtual ~VecIndex() { }

		inline VecIndex& operator=(TIt it) {
			*this = std::distance(std::begin(m_Vector), it);
			return *this;
		}
		inline VecIndex& operator=(IndexType i) {
			m_Index = i;
			return *this;
		}
		inline VecIndex& operator++() {
			Increase();
			return *this;
		}
		inline VecIndex operator++(int) {
			auto v = *this;
			++(*this);
			return v;
		}
		inline VecIndex& operator--() {
			Decrease();
		}
		inline VecIndex operator--(int) {
			auto v = *this;
			--(*this);
			return v;
		}
		inline VecIndex& operator+=(IndexType n) {
			Increase(n);
			return *this;
		}
		inline VecIndex& operator-=(IndexType n) {
			Decrease(n);
			return *this;
		}
		inline VecIndex operator+(IndexType n) const {
			auto v = *this;
			v.Increase(n);
			return v;
		}
		inline VecIndex operator-(IndexType n) const  {
			auto v = *this;
			v.Decrease(n);
			return v;
		}
		inline ValType& operator*() const { return *Ptr(); }
		inline ValType* operator->() const { return Ptr(); }
		inline bool operator==(const ValType& v) const { return Ptr() == v.Ptr(); }
		inline bool operator!=(const ValType& v) const { return !(*this == v); }
		inline operator bool() const { return OK(); }

		inline bool OK() const {
			return m_Vector != nullptr && m_Index < m_Vector->size();
		}
		inline ValType* Ptr() const {
			return OK() ? &(*m_Vector)[m_Index] : nullptr;
		}
		inline ValType& Get() const {
			return m_Vector->at(m_Index);
		}
		inline IndexType Index() const { return m_Index; }

	private:
		inline void Increase(size_t i = 1) {
			if (m_Vector) {
				if ((m_Index + i) >= m_Vector->size())
					m_Index = m_Vector->size();
				else
					m_Index += i;
			}
			else m_Index = 0;
		}
		inline void Decrease(size_t i = 1) {
			if (m_Vector && i < m_Index)
				m_Index -= i;
			else 
				m_Index = 0;
		}

		Vector* m_Vector = nullptr;
		IndexType m_Index = 0;
	};

	class line {
		std::string str;
	public:
		operator std::string() const { return str; }
		friend std::istream & operator>>(std::istream &is, line &l)
		{
			std::getline(is, l.str);
			return is;
		}
	};

	// returns how long the strings match for (in bytes, not time - but that would be cool)
	static inline size_t lengthcompare(const std::string& a, const std::string& b) {
		size_t l = 0;
		while (a.length() > l && b.length() > l && a[l] == b[l]) ++l;
		return l;
	}

	// makes your string real lazy
	static inline std::string tolower(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), [](int c){ return std::tolower(c); });
		return str;
	}
	// makes your string real shouty
	static inline std::string toupper(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), [](int c){ return std::toupper(c); });
		return str;
	}

	// trim from start
	static inline std::string &ltrim(std::string &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}
	// trim from end
	static inline std::string &rtrim(std::string &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
	}
	// trim from both ends (ltrim + rtrim, nothing special)
	static inline std::string &trim(std::string &s)
	{
		return ltrim(rtrim(s));
	}

	// copies while predicate returns true
	template<class InputIt, class UnaryPredicate>
	std::string strcpy_while(InputIt first, InputIt last, UnaryPredicate pred) {
		std::string str;
		while (first != last && pred(*first)) {
			str += *first++;
		}
		return str;
	}
	// strncpy + zero-terminate last character
	template<class T>
	inline T* szncpy(T *out, const char *src, size_t size)
	{
		strncpy(out, src, size);
		out[size - 1] = 0;
		return out;
	}

	template<typename T>
	inline T BytesToBits(T v) {
		return v * 8;
	}

	// I can't stress how often I've needed a program to be aware of the number of bits its data used...
	template<typename T>
	inline size_t CountBitOccupation(T N) {
		size_t r = 1;
		while (N >>= 1) r++;
		return r;
	}
	// can magically turn an arbitrary number into a number representing the amount of data in bytes the prior number used up
	template<typename T>
	inline size_t CountByteOccupation(T v) {
		size_t n = CountBitOccupation<T>(v);
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
	inline uint64_t GetFileModifiedTime(FILE *hFile)
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