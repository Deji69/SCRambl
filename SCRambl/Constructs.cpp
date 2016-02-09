#include "stdafx.h"
#include "Types.h"
#include "Constructs.h"
#include "Engine.h"
#include "Builder.h"

using namespace SCRambl;
using namespace SCRambl::Constructing;

void Constructs::Init(Build& build) {
	m_Config = build.AddConfig("Constructs");
	auto construct = m_Config->AddClass("Construct", [this](const XMLNode xml, void*& obj){
		auto construct = AddConstruct(xml["Name"]->AsString());
		obj = construct;
	});
	auto block = construct->AddClass("Block", [this](const XMLNode xml, void*& obj){
		auto& construct = *static_cast<SCRambl::Constructing::Construct*>(obj);
		auto begin = xml["Begin"]->AsString("{");
		auto end = xml["End"]->AsString("}");
		bool defaultMulti = begin == construct.Name();
		auto& block = construct.AddBlock(begin, end, xml["Multi"]->AsBool(defaultMulti));
		obj = &block;
	}, [this](const XMLNode xml, void*& obj){
		auto& block = *static_cast<SCRambl::Constructing::Block*>(obj);
		auto begin = xml["Begin"]->AsString("{");
		auto end = xml["End"]->AsString("}");
		auto& bloc = block.AddBlock(begin, end, xml["Multi"]->AsBool(false));
		obj = &bloc;
	});
	auto conditions = block->AddClass("Conditions", [this](const XMLNode xml, void*& obj){
		auto& block = *static_cast<SCRambl::Constructing::Block*>(obj);
		auto& conditionList = block.AddConditionList();
		if (auto min_attr = xml["Min"])
			conditionList.SetMinConditions(min_attr->AsNumber(0));
		if (auto max_attr = xml["Max"])
			conditionList.SetMaxConditions(max_attr->AsNumber(0));
		obj = &conditionList;
	});
	conditions->AddClass("DisableMixedLogic", [this](const XMLNode xml, void*& obj){
		auto& conditionList = *static_cast<SCRambl::Constructing::ConditionList*>(obj);
		conditionList.SetDisableMixedLogic(true);
	});
	AddDataConfig(block, "Data");
	//AddDataConfig(block, "End");
}
void Constructs::AddDataConfig(XMLConfig* config, const char* name) {
	auto data = config->AddClass(name, [this](const XMLNode xml, void*& obj){
		static std::map<std::string, DataPosition> map = {
			{ "Block", DataPosition::Block },
			{ "Conditions", DataPosition::Conditions },
		};

		auto& block = *static_cast<SCRambl::Constructing::Block*>(obj);

		XMLAttribute pos_attr;
		bool before = false;
		if (pos_attr = xml["Before"])
			before = true;
		else pos_attr = xml["After"];

		if (pos_attr) {
			auto it = map.find(pos_attr->AsString());
			if (it != map.end()) {
				auto& data = block.AddData(it->second, before);
				obj = &data;
				return;
			}
		}

		obj = nullptr;
	});
	data->AddClass("Command", [](const XMLNode xml, void*& obj){
		auto& data = *static_cast<SCRambl::Constructing::Data*>(obj);
		auto& code = data.AddCode<CommandCode>();
		if (xml["Name"])
			code.AddCondition(CommandCode::ConditionType::Name, xml["Name"]);
		if (*xml) {
			std::vector<std::string> args;
			if (tokenize(args, xml->AsString())) {
				for (auto& arg : args) {
					if (arg.size() <= 1) continue;
					if (arg[0] != '{') continue;
					
					auto i = arg.find('}', 1);
					if (i == arg.npos) continue;

					std::string name(arg, 1, i - 1);
					if (name.empty()) continue;
					for (size_t i = 1; i < arg.size(); ++i) {

					}
				}
			}
		}
		obj = &code;
	});
}
Constructs::Constructs()
{ }