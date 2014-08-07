/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include "utils.h"
#include "Configuration.h"
#include "SCR.h"

namespace SCRambl
{
	class Engine;

	/*\
	 * Types - SCR type manager
	\*/
	class Types
	{
	public:
		using Map = std::unordered_map < std::string, std::shared_ptr<SCR::Type> >;

	private:
		Engine							&	m_Engine;
		std::shared_ptr<Configuration>		m_Config;
		Map									m_Map;

	public:
		Types(Engine & eng);

		inline std::shared_ptr<SCR::Type> AddType(std::string name, unsigned long long id)
		{
			auto type = std::make_shared<SCR::Type>(id, name);
			m_Map.emplace(name, type);
			return type;
		}
		inline std::shared_ptr<SCR::Type> GetType(const std::string & name) const
		{
			if (m_Map.empty()) return nullptr;
			auto it = m_Map.find(name);
			if (m_Map.end() == it) return nullptr;
			return it->second;
		}
	};
}