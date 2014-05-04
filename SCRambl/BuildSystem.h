/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once

namespace SCRambl
{
	enum eBuildTask
	{
		BUILD_TASK_PRINT,
		BUILD_TASK_MKDIR,
	};

	class CBuildSystem
	{
		void echo(const char * msg);
		void copy(const char * src, const char * dst);
		void mkdir(const char * dir);
	};
}