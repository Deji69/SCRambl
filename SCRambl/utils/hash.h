/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <stdint.h>
#include <string>
#include "MurmurHash3.h"

namespace SCRambl
{
	inline uint_fast32_t GenerateHash(const std::string buf)
	{
		uint_fast32_t dw;
		MurmurHash3_x86_32(buf.c_str(), buf.length(), 0x88664422, &dw);
		return dw;
	}

	inline uint_fast32_t GenerateHash(const char *buf, size_t size)
	{
		uint_fast32_t dw;
		MurmurHash3_x86_32(buf, size, 0x88664422, &dw);
		return dw;
	}
}