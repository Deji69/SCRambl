#include "stdafx.h"
#include "Types.h"
#include "Commands.h"
#include "Engine.h"
#include "Builder.h"

namespace SCRambl
{
	// CommandArg
	CommandArg::CommandArg(Type* type, size_t index, bool isRet) : m_Type(type), m_Index(index), m_IsReturn(isRet)
	{ }

	// Command
	CommandArg& Command::GetArg(size_t i) { return m_Args[i]; }
	const CommandArg& Command::GetArg(size_t i) const { return m_Args[i]; }
	void Command::AddArg(Arg::Type* type, bool isRet) {
		m_Args.emplace_back(type, m_Args.size(), isRet);
	}
	Command::Command(std::string name, size_t index) : m_Name(name), m_Index(index)
	{ }

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

		m_Config = build.AddConfig("Commands");

		m_Config->AddClass("Case", [this, &usecc, &ccdest, &ccsrc](const XMLNode xml, void*& obj){
			usecc = xml.GetAttribute("Convert").GetValue().AsBool();
			ccdest = GetCasingByName(xml.GetAttribute("To").GetValue().AsString());
			ccsrc = GetCasingByName(xml.GetAttribute("From").GetValue().AsString());
		});
		auto args = m_Config->AddClass("Command", [this](const XMLNode xml, void*& obj){
			// store the command to the object pointer so we can access it again
			auto command = AddCommand(CaseConvert(xml.GetAttribute("Name").GetValue().AsString()), xml.GetAttribute("ID").GetValue());
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
	Command* Commands::AddCommand(std::string name, XMLValue id) {
		if (name.empty()) return nullptr;

		if (m_SourceCasing != m_DestCasing) {
			if (m_DestCasing != Casing::none)
				std::transform(name.begin(), name.end(), name.begin(), m_DestCasing == Casing::lowercase ? std::tolower : std::toupper);
		}

		m_Commands.emplace_back(name, id);
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