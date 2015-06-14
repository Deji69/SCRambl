/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "utils.h"
#include "Types.h"
#include "Variables.h"

namespace SCRambl
{
	enum class MatchLevel {
		nomatch, loose, basic, strict
	};

	class MatchSpec {
	public:
		
	};

	class Match {
	public:

	private:
		MatchLevel m_matchLevel = MatchLevel::nomatch;
	};
}