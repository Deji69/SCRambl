/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <map>
#include <cctype>

namespace SCRambl
{
	struct i_less : std::binary_function < std::string, std::string, bool >
	{
		struct nocase_compare : public std::binary_function < unsigned char, unsigned char, bool >
		{
			bool operator() (const unsigned char& c1, const unsigned char& c2) const {
				return std::tolower(c1) < std::tolower(c2);
			}
		};
		bool operator() (const std::string & s1, const std::string & s2) const {
			return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), nocase_compare());
		}
	};
}