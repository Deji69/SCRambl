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

	// CommandAttributes
	enum class CommandAttributeID {
		Name, ID, Conditional, NumArgs,
		None
	};
	class CommandAttributeSet : public AttributeSet<CommandAttributeID> {
	public:
		CommandAttributeSet() : AttributeSet(CommandAttributeID::None) {
			AddAttribute("ID", CommandAttributeID::ID);
			AddAttribute("Name", CommandAttributeID::Name);
			AddAttribute("NumArgs", CommandAttributeID::NumArgs);
			AddAttribute("Conditional", CommandAttributeID::Conditional);
		}
	};
	class CommandAttributes : public Attributes<CommandAttributeID, CommandAttributeSet> {
	public:
		CommandAttributes()
		{ }
	};

	// CommandArg - Consists simply of the expected type, the command index, and whether it's a return
	class CommandArg
	{
	public:
		using Type = Types::Type;
		using Vector = std::vector<CommandArg>;
		using Iterator = Vector::iterator;

		CommandArg(Type* type, size_t index, bool isRet = false, size_t size = 0);

		inline bool IsReturn() const { return m_IsReturn; }
		inline size_t GetIndex() const { return m_Index; }
		inline size_t GetSize() const { return m_Size; }
		inline Type* GetType() const { return m_Type; }

	private:
		Type* m_Type;
		size_t m_Index;						// nth arg
		bool m_IsReturn = false;
		size_t m_Size = 0;					// 0 = 'auto'
	};

	// Command - At the heart of SCR
	class Command
	{
	public:
		using Arg = CommandArg;
		using ArgVec = std::vector<Arg>;
		using Attributes = Attributes<Types::DataAttributeID, Types::DataAttributeSet>;
		using Ref = VecRef<Command>;

	private:
		XMLValue m_Index;			// index, which could be name/id/hash, depends on translation
		std::string m_Name;			// command name identifier
		ArgVec m_Args;				// arg vector me matey!
		Types::Type* m_Type;		// command type

	public:
		Command(std::string name, XMLValue index, Types::Type* type);

		Attributes GetAttributes() const;

		void AddArg(Arg::Type* type, bool isRet = false, size_t valueSize = 0);
		Arg& GetArg(size_t i);
		const Arg& Command::GetArg(size_t i) const;

		inline ArgVec::const_iterator BeginArg() const { return m_Args.begin(); }
		inline ArgVec::iterator BeginArg() { return m_Args.begin(); }
		inline ArgVec::const_iterator EndArg() const { return m_Args.end(); }
		inline ArgVec::iterator EndArg() { return m_Args.end(); }
		inline size_t NumArgs() const { return m_Args.size(); }
		inline XMLValue ID() const { return m_Index; }
		inline std::string Name() const { return m_Name; }
		inline Types::Type* Type() const { return m_Type; }
	};
	
	// CommandValue 
	class CommandValue : public Types::Value {
		Types::DataAttributeID m_ValueID;
		Types::DataType::Type m_ValueType;

	public:
		CommandValue(Types::Type* type, size_t size, std::string valueid, Types::DataType datatype) : Value(type, Types::ValueSet::Command, size),
			m_ValueID(Types::DataAttributeID::None),
			m_ValueType(datatype)
		{ }

		size_t GetValueSize(const Command::Attributes&) const;
	};

	// Commands - SCR command manager
	class Commands {
	public:
		enum class Casing {
			none, uppercase, lowercase
		};

		using Map = std::unordered_multimap<std::string, size_t>;
		using Vector = std::vector<Command::Ref>;

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
		Command::Ref AddCommand(std::string name, XMLValue id, Types::Type*);
		Command::Ref GetCommand(size_t index);
		
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