/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include <unordered_map>
#include "utils.h"
#include "Configuration.h"
#include "SCR.h"

namespace SCRambl
{
	class Engine;

	/*\
	 * Commands - SCR command manager
	\*/
	class Commands
	{
	public:
		using Map = std::unordered_multimap < std::string, std::shared_ptr<SCR::Command> > ;

	private:
		Engine							&	m_Engine;
		std::shared_ptr<Configuration>		m_Config;
		Map									m_Map;

	public:
		Commands(Engine & engine);

		void Init();

		std::shared_ptr<SCR::Command> AddCommand(std::string name, unsigned long long opcode)
		{
			auto command = std::make_shared<SCR::Command>(name, opcode);
			m_Map.emplace(name, command);
			return command;
		}
	};
}