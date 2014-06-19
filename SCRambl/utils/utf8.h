#pragma once
#ifdef _WIN32
	#include <Windows.h>
#endif

namespace SCRambl
{
	/*inline std::wstring widen(const char * str)
	{
		std::wstring out;
		for (char c; c = *str; ++str)
		{
			out.push_back(std::cout.widen(c));
		}
		return out;
	}

	inline std::string narrow(const wchar_t * str)
	{
		std::string out;
		for (wchar_t c; c = *str; ++str)
		{
			out.push_back(std::cout.narrow(c, '?'));
		}
		return out;
	}

	inline std::wstring widen(const std::string& str)
	{
		std::wstring out;
		for (char c : str)
		{
			out.push_back(std::cout.widen(c));
		}
		return out;
	}

	inline std::string narrow(const std::wstring& str)
	{
		std::string out;
		for (char c : str)
		{
			out.push_back(std::cout.narrow(c, '?'));
		}
		return out;
	}*/

#ifdef _WIN32
	inline std::wstring widen(const char * str)
	{
		size_t n = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
		std::wstring out;
		out.resize(n);
		MultiByteToWideChar(CP_UTF8, 0, str, -1, &out[0], n);
		return out;
	}

	inline std::string narrow(const wchar_t * str)
	{
		size_t n = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
		std::string out;
		out.resize(n);
		WideCharToMultiByte(CP_UTF8, 0, str, -1, &out[0], n, NULL, nullptr);
		return out;
	}

	inline std::wstring widen(const std::string& str)
	{
		size_t n = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), nullptr, 0);
		std::wstring out;
		out.resize(n);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), &out[0], n);
		return out;
	}

	inline std::string narrow(const std::wstring& str)
	{
		size_t n = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.size(), nullptr, 0, nullptr, nullptr);
		std::string out;
		out.resize(n);
		WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.size(), &out[0], n, NULL, nullptr);
		return out;
	}
	
#else

	static wchar_t wchar_buffer[5000];
	static char char_buffer[5000];

	inline std::wstring widen(const char * str)
	{
		size_t n = mbstowcs(wchar_buffer, str, sizeof(wchar_buffer));
		std::wstring out;
		out.resize(n);
		mbstowcs(&out[0], str, n);
		return out;
	}

	inline std::string narrow(const wchar_t * str)
	{
		size_t n = wcstombs(char_buffer, str, sizeof(char_buffer));
		std::string out;
		out.resize(n);
		wcstombs(&out[0], str, n);
		return out;
	}

	inline std::wstring widen(const std::string& str)
	{
		size_t n = mbstowcs(wchar_buffer, str.c_str(), sizeof(char_buffer));
		std::wstring out;
		out.resize(n);
		mbstowcs(&out[0], str.c_str(), str.size());
		return out;
	}

	inline std::string narrow(const std::wstring& str)
	{
		size_t n = wcstombs(char_buffer, str.c_str(), str.size());
		std::string out;
		out.resize(n);
		wcstombs(&out[0], str.c_str(), str.size());
		return out;
	}
#endif
}