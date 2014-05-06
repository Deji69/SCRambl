/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Builder.h"
#include <list>

namespace SCRambl
{
	class Engine
	{
		Builder				*	m_pBuild;

	public:
		Engine();

		RunningState Run();
	};
}