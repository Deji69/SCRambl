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
#include "Constructs.h"
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

		CommandArg(VecRef<Type> type, size_t index, bool isRet = false, size_t size = 0);
		virtual ~CommandArg() = default;

		inline bool IsReturn() const { return m_IsReturn; }
		inline size_t GetIndex() const { return m_Index; }
		inline size_t GetSize() const { return m_Size; }
		inline Type* GetType() const { return m_Type.Ptr(); }

	private:
		VecRef<Type> m_Type;
		size_t m_Index;						// nth arg
		bool m_IsReturn = false;
		size_t m_Size = 0;					// 0 = 'auto'
	};
	//
	class CommandVarArg : public CommandArg
	{
	public:
		using Vector = std::vector<CommandVarArg>;
		using Iterator = Vector::iterator;

		CommandVarArg(VecRef<Type> type, size_t index, bool isRet = false, size_t size = 0, size_t max = 0, size_t min = 0);

		inline size_t GetArgMinimum() const { return m_Minimum; }
		inline size_t GetArgMaximum() const { return m_Maximum; }

	private:
		size_t	m_Minimum,
				m_Maximum;
	};
	//
	class CommandAutoArg : public CommandArg
	{
	public:
		using Vector = std::vector<CommandAutoArg>;
		using Iterator = Vector::iterator;

		CommandAutoArg(VecRef<Type> type, size_t index, Types::DataSource src, Types::DataAttribute attr, bool isRet = false, size_t size = 0);

		inline Types::DataSource GetSource() const { return m_Source; }
		inline Types::DataAttribute GetAttribute() const { return m_Attribute; }

	private:
		Types::DataSource m_Source;
		Types::DataAttribute m_Attribute;
	};

	// Command - At the heart of SCR
	class Command
	{
	public:
		using Arg = CommandArg;
		using VarArg = CommandVarArg;
		using AutoArg = CommandAutoArg;
		using ArgVec = std::vector<Arg>;
		using VarArgVec = std::vector<VarArg>;
		using AutoArgVec = std::vector<AutoArg>;
		using Attributes = Attributes<Types::DataAttributeID, Types::DataAttributeSet>;
		using Ref = VecRef<Command>;
		
		struct NoArgsConfig {
			bool m_DisableTranslation = false;
		};
		class VarArgsConfig {
		public:
			VarArgsConfig() = default;
			~VarArgsConfig() = default;

			template<typename... TArgs>
			inline void AddVarArg(TArgs&&... args) { m_VarArgs.emplace_back(args); }
			template<typename... TArgs>
			inline void AddAutoArg(TArgs&&... args) { m_AutoArgs.emplace_back(args); }
			inline void SetIndex(XMLValue xml) { m_Index = xml; }
			inline void SetType(Types::TypeRef<Types::Type> type) { m_Type = type; }
			
		private:
			XMLValue m_Index;
			Types::TypeRef<Types::Type> m_Type;
			VarArgVec m_VarArgs;
			AutoArgVec m_AutoArgs;
		};

	private:
		XMLValue m_Index;									// index, which could be name/id/hash, depends on translation
		std::string m_Name;									// command name identifier
		ArgVec m_Args;										// arg vector me matey!
		VecRef<Types::Type> m_Type;							// command type
		bool m_DisableCall = false,							// disallow direct calling
			 m_Disable = false,								// disable command
			 m_DisableTranslation = false;					// disable translation/output
		NoArgsConfig m_NoArgsConfig;
		std::unique_ptr<VarArgsConfig> m_VarArgsConfig;
		Constructing::Construct* m_Construct = nullptr;		// start construct

	public:
		Command(std::string name, XMLValue index, VecRef<Types::Type> type);
		Command(const Command&) = delete;
		Command(Command&& v) : m_Index(v.m_Index), m_Name(v.m_Name), m_Args(v.m_Args), m_Type(v.m_Type),
			m_DisableCall(v.m_DisableCall), m_Disable(v.m_Disable), m_NoArgsConfig(v.m_NoArgsConfig),
			m_VarArgsConfig(std::move(v.m_VarArgsConfig))
		{ }

		Attributes GetAttributes() const;

		void AddArg(VecRef<Arg::Type> type, bool isRet = false, size_t valueSize = 0);
		Arg& GetArg(size_t i);
		const Arg& Command::GetArg(size_t i) const;

		inline ArgVec::const_iterator BeginArg() const { return m_Args.begin(); }
		inline ArgVec::iterator BeginArg() { return m_Args.begin(); }
		inline ArgVec::const_iterator EndArg() const { return m_Args.end(); }
		inline ArgVec::iterator EndArg() { return m_Args.end(); }
		inline size_t NumParams() const { return m_Args.size(); }
		inline size_t NumRequiredArgs() const { return m_Args.size(); }
		inline XMLValue ID() const { return m_Index; }
		inline std::string Name() const { return m_Name; }
		inline Types::Type* Type() const { return m_Type.Ptr(); }
		inline NoArgsConfig& GetNoArgsConfig() { return m_NoArgsConfig; }
		inline VarArgsConfig& GetVarArgsConfig() { if (!m_VarArgsConfig) { m_VarArgsConfig = std::make_unique<VarArgsConfig>(); } return *m_VarArgsConfig; }
		inline bool IsDisabled() const { return m_Disable; }
		inline bool IsCallDisabled() const { return m_DisableCall; }
		inline bool IsTranslationDisabled() const { return m_DisableTranslation; }
		inline Constructing::Construct* GetConstruct() const { return m_Construct; }
		inline void SetDisabled(bool v) { m_Disable = v; }
		inline void SetDisableCall(bool v) { m_DisableCall = v; }
		inline void SetDisableTranslation(bool v) { m_DisableTranslation = v; }
		inline void SetConstruct(Constructing::Construct* construct) { m_Construct = construct; }
	};
	
	// CommandValue 
	class CommandValue : public Types::Value {
		Types::DataAttributeID m_ValueID;
		Types::DataType::Type m_ValueType;

	public:
		CommandValue(Types::Storage& types, size_t type_idx, size_t size, std::string valueid, Types::DataType datatype, Types::ValueSet valtype = Types::ValueSet::Command) : Value(types, type_idx, valtype, size),
			m_ValueID(Types::DataAttributeID::None),
			m_ValueType(datatype)
		{ }
		virtual ~CommandValue() = default;

		inline Types::DataAttributeID GetAttributeID() const { return m_ValueID; }
		inline Types::DataType::Type GetDataType() const { return m_ValueType; }
		size_t GetValueSize(const Command::Attributes&) const;
	};

	// Commands - SCR command manager
	class Commands {
	public:
		enum class Casing {
			none, uppercase, lowercase
		};

		using Map = std::unordered_multimap < std::string, size_t > ;
		using Vector = std::vector < Command::Ref > ;

	private:
		XMLConfiguration* m_Config;
		bool m_UseCaseConversion;
		Casing m_SourceCasing = Casing::none;
		Casing m_DestCasing = Casing::none;
		Map	m_Map;
		std::vector<Command> m_Commands;
		std::unordered_map<std::string, Command::Ref> m_CommandConstructs;
		bool m_CommandConstructsResolved = true;

	public:
		Commands() = default;

		void Init(Build& build);
		std::string CaseConvert(std::string) const;
		Command::Ref AddCommand(std::string name, XMLValue id, VecRef<Types::Type>);
		Command::Ref GetCommand(size_t index);

		// Finds all commands matching the name and stores them in a passed vector of command handles, excluding call-disabled
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
				auto command = GetCommand(it->second);
				if (command->IsDisabled()) continue;
				if (func(GetCommand(command.Index())))
					++num;
			}
			return num;
		}

		void ResolveCommandConstructs(const Constructing::Constructs& constructs) {
			if (!m_CommandConstructsResolved) {
				for (auto pr : m_CommandConstructs) {
					auto construct = constructs.GetConstruct(pr.first);
					if (!construct) {
						BREAK();				// error: construct not found for command
					}
					else {
						pr.second->SetConstruct(construct);
					}
				}

				m_CommandConstructs.clear();
				m_CommandConstructsResolved = true;
			}
		}

		// Get casing by name - or rather, the first character - whatever
		static Casing GetCasingByName(std::string);

	private:
		void AddCommandConstruct(std::string name, Command::Ref command) {
			m_CommandConstructs.emplace(name, command);
			m_CommandConstructsResolved = false;
		}
	};
}