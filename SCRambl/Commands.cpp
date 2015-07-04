#include "stdafx.h"
#include "Types.h"
#include "Commands.h"
#include "Engine.h"
#include "Builder.h"

namespace SCRambl
{
	const std::map<std::string, CommandAttributeID> CommandAttributes::map = {
		{ "Name", CommandAttributeID::Name },
		{ "ID", CommandAttributeID::ID },
		{ "Conditional", CommandAttributeID::Conditional },
		{ "NumArgs", CommandAttributeID::NumArgs }
	};
	const AttributeSet<CommandAttributeID> CommandAttributes::attribute_set = { CommandAttributeID::None, map };

	// CommandArg
	CommandArg::CommandArg(Type* type, size_t index, bool isRet) : m_Type(type), m_Index(index), m_IsReturn(isRet)
	{ }
	
	// CommandValue
	size_t CommandValue::GetValueSize(const CommandAttributes* cmd) const {
		switch (m_ValueType) {
		case Types::DataType::Int:
			return CountBitOccupation(cmd->GetAttribute(m_ValueID).AsNumber<long long>());
		case Types::DataType::String:
			return cmd->GetAttribute(m_ValueID).AsString().size() * 8;
		case Types::DataType::Float:
			return sizeof(float);
		case Types::DataType::Char:
			return sizeof(char);
		default: BREAK();
		}
		return 0;
	}

	// Command
	CommandArg& Command::GetArg(size_t i) { return m_Args[i]; }
	const CommandArg& Command::GetArg(size_t i) const { return m_Args[i]; }
	void Command::AddArg(Arg::Type* type, bool isRet) {
		m_Args.emplace_back(type, m_Args.size(), isRet);
		SetAttribute(CommandAttributeID::NumArgs, m_Args.size());
	}
	Command::Command(std::string name, XMLValue index, Types::Type* type) : m_Name(name), m_Index(index), m_Type(type)
	{
		SetAttribute(CommandAttributeID::Name, name);
		SetAttribute(CommandAttributeID::ID, index);
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
		Types::Type* type = nullptr;

		m_Config = build.AddConfig("Commands");

		m_Config->AddClass("Case", [this, &usecc, &ccdest, &ccsrc](const XMLNode xml, void*& obj){
			usecc = xml.GetAttribute("Convert").GetValue().AsBool();
			ccdest = GetCasingByName(xml.GetAttribute("To").GetValue().AsString());
			ccsrc = GetCasingByName(xml.GetAttribute("From").GetValue().AsString());
		});
		m_Config->AddClass("CommandType", [this, &type, &types](const XMLNode xml, void*& obj){
			type = types.GetType(xml["Type"]->AsString());
			if (!type) BREAK();
		});
		auto args = m_Config->AddClass("Command", [this, &type, &types](const XMLNode xml, void*& obj){
			// store the command to the object pointer so we can access it again
			auto stype = type;
			if (auto type_attr = xml.GetAttribute("Type")) {
				stype = types.GetType(type_attr->AsString());
			}
			auto command = AddCommand(CaseConvert(xml["Name"]->AsString()), *xml["ID"], stype);
			obj = command;
		})->AddClass("Args");
		args->AddClass("Arg", [this, &types](const XMLNode xml, void*& obj){
			// retrieve the object poiter as a SCR command we know it to be
			auto& command = *static_cast<SCRambl::Command*>(obj);
			if (auto type = types.GetType(xml.GetAttribute("Type").GetValue().AsString())) {
				command.AddArg(static_cast<Types::Type*>(type), xml.GetAttribute("Out").GetValue().AsBool());
			}
			else {
				auto name = xml.GetAttribute("Type").GetValue().AsString();
				name.size();
			}
		});
	}
	Command* Commands::GetCommand(size_t index) {
		return index >= m_Commands.size() ? nullptr : &m_Commands[index];
	}
	Command* Commands::AddCommand(std::string name, XMLValue id, Types::Type* type) {
		if (name.empty()) return nullptr;

		if (m_SourceCasing != m_DestCasing) {
			if (m_DestCasing != Casing::none)
				std::transform(name.begin(), name.end(), name.begin(), m_DestCasing == Casing::lowercase ? std::tolower : std::toupper);
		}

		m_Commands.emplace_back(name, id, type);
		auto ptr = &m_Commands.back();
		m_Map.emplace(name, m_Commands.size() - 1);
		return ptr;
	}
	long Commands::FindCommands(std::string name, Vector& vec) {
		return ForCommandsNamed(name, [&vec](Command* ptr){ vec.push_back(ptr); });
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