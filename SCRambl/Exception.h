/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <exception>
#include <string>

namespace SCRambl
{
	class Exception : virtual public std::exception
	{
	public:
		Exception()
		{
		}
		Exception(const char * msg) : std::exception(msg)
		{
		}
		Exception(const std::string & str) : std::exception(str.c_str())
		{
		}
		Exception(const std::exception & excep) : std::exception(excep.what())
		{
		}

		inline const char * Message() const			{ return what(); }
	};
}