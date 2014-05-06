/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "States.h"

namespace SCRambl
{
	class Preprocessor : public PreprocessorState
	{
	public:
		void RunTask(RunningState &) override;
	};
}