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
#include "Types.h"
#include "Values.h"

namespace SCRambl
{
	class Engine;
	class Build;

	class CommandArg
	{
		Types::Type* m_Type;
		size_t m_Index;						// nth arg
		bool m_IsReturn = false;

	public:
		using Type = Types::Type;
		using Vector = std::vector<CommandArg>;
		using Iterator = Vector::iterator;

		CommandArg(Type* type, size_t index, bool isRet = false) : m_Type(type), m_Index(index), m_IsReturn(isRet)
		{ }

		inline bool IsReturn() const { return m_IsReturn; }
		inline size_t GetIndex() const { return m_Index; }
		inline Type* GetType() const { return m_Type; }
	};

	class Command
	{
	public:
		typedef CommandArg Arg;
		typedef std::vector<Arg> ArgVector;

	private:
		size_t m_Index;				// unique index/hash
		std::string m_Name;			// command name/id
		ArgVector m_Args;

	public:
		Command(std::string name, size_t index) : m_Name(name), m_Index(index)
		{ }

		void AddArg(Arg::Type* type, bool isRet = false) {
			m_Args.emplace_back(type, m_Args.size(), isRet);
		}

		inline Arg& GetArg(size_t i) { return m_Args[i]; }
		inline const Arg& GetArg(size_t i) const { return m_Args[i]; }

		inline ArgVector::const_iterator BeginArg() const { return m_Args.begin(); }
		inline ArgVector::iterator BeginArg() { return m_Args.begin(); }
		inline ArgVector::const_iterator EndArg() const	{ return m_Args.end(); }
		inline ArgVector::iterator EndArg() { return m_Args.end(); }
		inline size_t GetNumArgs() const { return m_Args.size(); }
		inline std::string GetName() const { return m_Name; }
	};

	/*\ Commands - SCR command manager \*/
	class Commands
	{
	public:
		enum class Casing {
			none, uppercase, lowercase
		};

		using Map = std::unordered_multimap<std::string, Command*>;
		using Vector = std::vector<Command*>;

	private:
		XMLConfiguration* m_Config;
		bool m_UseCaseConversion;
		Casing m_SourceCasing = Casing::none;
		Casing m_DestCasing = Casing::none;
		Map	m_Map;
		std::vector<Command> m_Commands;

	public:
		Commands();

		void Init(Build& build);

		// Get casing by name - or rather, the first character - whatever
		inline Casing GetCasingByName(std::string name) const {
			if (!name.empty()) {
				if (name[0] == 'u' || name[0] == 'U') return Casing::uppercase;
				if (name[0] == 'l' || name[0] == 'L') return Casing::lowercase;
			}
			return Casing::none;
		}

		inline Command* AddCommand(std::string name, size_t opcode) {
			if (name.empty()) return nullptr;

			if (m_SourceCasing != m_DestCasing) {
				if (m_DestCasing != Casing::none)
					std::transform(name.begin(), name.end(), name.begin(), m_DestCasing == Casing::lowercase ? std::tolower : std::toupper);
			}

			m_Commands.emplace_back(name, opcode);
			auto ptr = &m_Commands.back();
			m_Map.emplace(name, ptr);
			return ptr;
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
		inline long FindCommands(std::string name, Vector& vec) {
			return ForCommandsNamed(name, [&vec](Command* ptr){ vec.push_back(ptr); });
		}
	};
}