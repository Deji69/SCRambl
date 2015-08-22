#include "stdafx.h"
#include "Types.h"
#include "Commands.h"
#include "Engine.h"
#include "Builder.h"

namespace SCRambl
{
	/*const std::map<std::string, CommandAttributeID> CommandAttributes::map = {
		{ "Name", CommandAttributeID::Name },
		{ "ID", CommandAttributeID::ID },
		{ "Conditional", CommandAttributeID::Conditional },
		{ "NumArgs", CommandAttributeID::NumArgs }
	};
	const AttributeSet<CommandAttributeID> CommandAttributes::attribute_set = { CommandAttributeID::None, map };*/

	const CommandAttributeSet Attributes<CommandAttributeID, CommandAttributeSet>::s_AttributeSet;

	// CommandArg
	CommandArg::CommandArg(VecRef<Type> type, size_t index, bool isRet, size_t size) : m_Type(type), m_Index(index), m_IsReturn(isRet), m_Size(size)
	{ }
	
	// CommandValue
	size_t CommandValue::GetValueSize(const Command::Attributes& cmd) const {
		switch (m_ValueType) {
		case Types::DataType::Int:
			return CountBitOccupation(cmd.GetAttribute(m_ValueID).AsNumber<long long>());
		case Types::DataType::String:
			return cmd.GetAttribute(m_ValueID).AsString().size() * 8;
		case Types::DataType::Float:
			return sizeof(float);
		case Types::DataType::Char:
			return sizeof(char);
		default: BREAK();
		}
		return 0;
	}

	// Command
	Command::Attributes Command::GetAttributes() const {
		Attributes attr;
		attr.SetAttribute(Types::DataAttributeID::ID, m_Index);
		attr.SetAttribute(Types::DataAttributeID::Name, m_Name);
		attr.SetAttribute(Types::DataAttributeID::NumArgs, m_Args.size());
		return attr;
	}
	CommandArg& Command::GetArg(size_t i) { return m_Args[i]; }
	const CommandArg& Command::GetArg(size_t i) const { return m_Args[i]; }

	void Command::AddArg(VecRef<Arg::Type> type, bool isRet, size_t size) {
		m_Args.emplace_back(type, m_Args.size(), isRet, size);
	}
	Command::Command(std::string name, XMLValue index, VecRef<Types::Type> type) : m_Name(name), m_Index(index), m_Type(type)
	{
		if (!m_Type) BREAK();
	}

	// Commands
	Commands::Casing Commands::GetCasingByName(std::string name) {
		if (!name.empty()) {
			if (name[0] == 'u' || name[0] == 'U') return Casing::uppercase;
			if (name[0] == 'l' || name[0] == 'L') return Casing::lowercase;
		}
		return Casing::none;
	}
	void Commands::Init(Build& build) {
		auto& types = build.GetTypes();
		auto& usecc = m_UseCaseConversion;
		auto& ccdest = m_DestCasing;
		auto& ccsrc = m_SourceCasing;
		VecRef<Types::Type> type;

		m_Config = build.AddConfig("Commands");

		m_Config->AddClass("Case", [this, &usecc, &ccdest, &ccsrc](const XMLNode xml, void*& obj){
			usecc = xml.GetAttribute("Convert").GetValue().AsBool();
			ccdest = GetCasingByName(xml.GetAttribute("To").GetValue().AsString());
			ccsrc = GetCasingByName(xml.GetAttribute("From").GetValue().AsString());
		});
		m_Config->AddClass("CommandType", [this, &type, &types](const XMLNode xml, void*& obj){
			type = types.GetType(xml["Type"]->AsString()).Ref();
			if (!type) BREAK();
		});
		auto args = m_Config->AddClass("Command", [this, &type, &types](const XMLNode xml, void*& obj){
			// store the command to the object pointer so we can access it again
			auto stype = type;
			if (auto type_attr = xml.GetAttribute("Type")) {
				stype = types.GetType(type_attr->AsString()).Ref();
			}
			auto command = AddCommand(CaseConvert(xml["Name"]->AsString()), *xml["ID"], stype);
			obj = command.Ptr();
		})->AddClass("Args");
		args->AddClass("Arg", [this, &types](const XMLNode xml, void*& obj){
			// retrieve the object poiter as a SCR command we know it to be
			auto& command = *static_cast<SCRambl::Command*>(obj);
			if (auto type = types.GetType(xml.GetAttribute("Type").GetValue().AsString())) {
				command.AddArg(type.Ref(), xml.GetAttribute("Out").GetValue().AsBool());
			}
			else {
				auto name = xml.GetAttribute("Type").GetValue().AsString();
				name.size();
			}
		});
	}
	Command::Ref Commands::GetCommand(size_t index) {
		return Command::Ref(m_Commands, index);
	}
	Command::Ref Commands::AddCommand(std::string name, XMLValue id, VecRef<Types::Type> type) {
		if (name.empty()) return nullptr;

		if (m_SourceCasing != m_DestCasing) {
			if (m_DestCasing != Casing::none)
				std::transform(name.begin(), name.end(), name.begin(), m_DestCasing == Casing::lowercase ? std::tolower : std::toupper);
		}

		m_Commands.emplace_back(name, id, type);
		m_Map.emplace(name, m_Commands.size() - 1);
		return { m_Commands, m_Commands.size() - 1 };
	}
	long Commands::FindCommands(std::string name, Vector& vec) {
		return ForCommandsNamed(name, [&vec](Command::Ref ptr){ vec.push_back(ptr); });
	}
	std::string Commands::CaseConvert(std::string str) const {
		if (m_UseCaseConversion) {
			if (m_DestCasing != Casing::none)
				std::transform(str.begin(), str.end(), str.begin(), m_DestCasing == Casing::lowercase ? std::tolower : std::toupper);
		}
		return str;
	}
	Commands::Commands()
	{ }
}