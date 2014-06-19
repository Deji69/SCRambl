/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <vector>
#include <string>

namespace SCRambl
{
	namespace Reporting
	{
		class Report
		{
			enum Type {
				notice, warning, error, fatal
			};
		};

		class Error : Report
		{

		};
	}
	class Error
	{
		unsigned long				m_nCode;
		std::vector<std::string>	m_Params;

	public:
		Error(unsigned long code, std::vector<std::string> params);
	};
}