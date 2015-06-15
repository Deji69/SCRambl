/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <set>
#include <unordered_set>
#include "utils.h"
#include "Types.h"
#include "Variables.h"

namespace SCRambl
{
	enum class MatchLevel {
		nomatch, loose, basic, strict
	};

	class TypeMatch {
	public:

	private:
		MatchLevel m_matchLevel = MatchLevel::nomatch;
	};

	class MatchSet {
	public:

	private:
	};
}