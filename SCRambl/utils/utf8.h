#pragma once
#include <Windows.h>

namespace SCRambl
{
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
}