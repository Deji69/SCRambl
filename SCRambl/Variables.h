/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <memory>
#include "SCR.h"

namespace SCRambl
{
	template<typename T>
	class Variable : public SCR::Variable<T>
	{
	public:
		using Shared = std::shared_ptr<Variable>;

		Variable(std::string name, typename T::Shared type) : SCR::Variable<T>(name, type)
		{ }
	};
}