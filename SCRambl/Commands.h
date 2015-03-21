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
#include "Types.h"

namespace SCRambl
{
	class Engine;

	// Use SCR Command's and SCR CommandArg's with our own TypeExt's
	typedef SCR::Command<SCR::CommandArg<Types::Type>> Command;

	/*\
	 * Commands - SCR command manager
	\*/
	class Commands
	{
	public:
		enum class Casing {
			none, uppercase, lowercase
		};

		using Map = std::unordered_multimap < std::string, Command::Shared >;
		using Vector = std::vector < Command::Shared >;

	private:
		Engine							&	m_Engine;
		std::shared_ptr<Configuration>		m_Config;
		bool								m_UseCaseConversion;
		Casing								m_SourceCasing = Casing::none;
		Casing								m_DestCasing = Casing::none;
		Map									m_Map;

	public:
		Commands(Engine & engine);

		// Get casing by name - or rather, the first character - whatever
		inline Casing GetCasingByName(std::string name) const {
			if (!name.empty()) {
				if (name[0] == 'u' || name[0] == 'U') return Casing::uppercase;
				if (name[0] == 'l' || name[0] == 'L') return Casing::lowercase;
			}
			return Casing::none;
		}

		inline std::shared_ptr<Command> AddCommand(std::string name, unsigned long long opcode) {
			if (name.empty()) return nullptr;

			if (m_SourceCasing != m_DestCasing) {
				if (m_DestCasing != Casing::none)
					std::transform(name.begin(), name.end(), name.begin(), m_DestCasing == Casing::lowercase ? std::tolower : std::toupper);
			}

			auto command = std::make_shared<Command>(name, opcode);
			m_Map.emplace(name, command);
			return command;
		}

		// Passes each command handle matching the name to the supplied function
		// Returns the number of calls / found commands
		template<typename TFunc>
		inline long ForCommandsNamed(std::string name, TFunc func) {
			if (m_UseCaseConversion)
			{
				if (m_DestCasing != Casing::none)
					std::transform(name.begin(), name.end(), name.begin(), m_DestCasing == Casing::lowercase ? std::tolower : std::toupper);
			}

			auto rg = m_Map.equal_range(name);
			if (rg.first == m_Map.end()) return 0;
			long num = 0;
			for (auto it = rg.first; it != rg.second; ++it) {
				func(it->second);
				++num;
			}
			return num;
		}
		
		// Finds all commands matching the name and stores them in a passed vector of command handles
		// Returns the number of commands found
		inline long FindCommands(std::string name, Vector & vec) {
			return ForCommandsNamed(name, [&vec](Command::Shared ptr){ vec.push_back(ptr); });
		}
	};
}