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
	public:
		using Type = Types::Type;
		using Vector = std::vector<CommandArg>;
		using Iterator = Vector::iterator;

		CommandArg(Type* type, size_t index, bool isRet = false);

		inline bool IsReturn() const { return m_IsReturn; }
		inline size_t GetIndex() const { return m_Index; }
		inline Type* GetType() const { return m_Type; }

	private:
		Type* m_Type;
		size_t m_Index;						// nth arg
		bool m_IsReturn = false;
	};

	class Command
	{
	public:
		using Arg = CommandArg;
		using ArgVec = std::vector<Arg>;

	private:
		size_t m_Index;				// unique index/hash
		std::string m_Name;			// command name/id
		ArgVec m_Args;				// Arg vector me matey!

	public:
		Command(std::string name, size_t index);

		void AddArg(Arg::Type* type, bool isRet = false);
		Arg& GetArg(size_t i);
		const Arg& Command::GetArg(size_t i) const;

		inline ArgVec::const_iterator BeginArg() const { return m_Args.begin(); }
		inline ArgVec::iterator BeginArg() { return m_Args.begin(); }
		inline ArgVec::const_iterator EndArg() const { return m_Args.end(); }
		inline ArgVec::iterator EndArg() { return m_Args.end(); }
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

		using Map = std::unordered_multimap<std::string, size_t>;
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
		std::string CaseConvert(std::string) const;
		Command* AddCommand(std::string name, XMLValue id);
		
		// Finds all commands matching the name and stores them in a passed vector of command handles
		// Returns the number of commands found
		long FindCommands(std::string name, Vector& out);

		// Passes each command handle matching the name to the supplied function
		// Returns the number of calls / found commands
		template<typename TFunc>
		inline long ForCommandsNamed(std::string name, TFunc func) {
			name = CaseConvert(name);

			auto rg = m_Map.equal_range(name);
			if (rg.first == m_Map.end()) return 0;
			long num = 0;
			for (auto it = rg.first; it != rg.second; ++it) {
				func(GetCommand(it->second));
				++num;
			}
			return num;
		}
		
		// Get casing by name - or rather, the first character - whatever
		static Casing GetCasingByName(std::string);
	};
}