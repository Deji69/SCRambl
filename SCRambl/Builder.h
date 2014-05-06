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
	class Builder : public BuilderState
	{
		void Init();

	protected:
		void RunTask() override;
	};
}