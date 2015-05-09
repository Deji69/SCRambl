#include "stdafx.h"
#include "utils.h"
#include "utils\MurmurHash3.h"
#include "utils\hash.h"

namespace SCRambl
{
	const VersionStruct SCRamblVersion = { 1, 0, 0, 0 };

	std::string VersionStruct::GetString() const {
		std::string ver = std::to_string(Major);
		if (Alpha) ver += "." + std::to_string(Minor) + "." + std::to_string(Beta) + "." + std::to_string(Alpha);
		else if (Beta) ver += "." + std::to_string(Minor) + "." + std::to_string(Beta);
		else if (Minor) ver += "." + std::to_string(Minor);
		return ver;
	}

	/*static uint_fast32_t GenerateHash(const std::string buf)
	{
		uint_fast32_t dw;
		MurmurHash3_x86_32(buf.c_str(), buf.length(), 0x88664422, &dw);
		return dw;
	}

	static uint_fast32_t GenerateHash(const char *buf, size_t size)
	{
		uint_fast32_t dw;
		MurmurHash3_x86_32(buf, size, 0x88664422, &dw);
		return dw;
	}*/
}