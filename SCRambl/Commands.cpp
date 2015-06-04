#include "stdafx.h"
#include "Types.h"
#include "Commands.h"
#include "Engine.h"
#include "Builder.h"

namespace SCRambl
{
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
		auto& args = m_Config->AddClass("Command", [this](const XMLNode xml, void*& obj){
			// store the command to the object pointer so we can access it again
			auto command = AddCommand(xml.GetAttribute("Name").GetValue().AsString(), xml.GetAttribute("ID").GetValue().AsNumber<size_t>());
			obj = command;
		}).AddClass("Args");
		args.AddClass("Arg", [this, &types](const XMLNode xml, void*& obj){
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
	Commands::Commands()
	{ }
}