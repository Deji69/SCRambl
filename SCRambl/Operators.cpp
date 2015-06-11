#include "stdafx.h"
#include "Operators.h"
#include "Builder.h"

namespace SCRambl
{
	namespace Operators
	{
		void Operators::Init(Build& build) {
			auto& types = build.GetTypes();

			m_Config = build.AddConfig("Operators");

			auto operater = m_Config->AddClass("Operator", [this](const XMLNode xml, void*& obj){
				// add operator
				auto name = xml.GetAttribute("Name").GetValue();
				auto not_attr = xml.GetAttribute("Not").GetValue();
				if (name) {
					auto op = Add(name.AsString(), not_attr.AsBool());
					if (op) {
						// map pseudonyms
						auto cond_attr = xml.GetAttribute("Cond").GetValue();
						auto comp_attr = xml.GetAttribute("Comp").GetValue();
						if (comp_attr) Add(comp_attr.AsString(), op);
						if (not_attr) Add(not_attr.AsString(), op);
					}
					obj = op.Ptr();
				}
				else obj = nullptr;
			});
			operater->AddClass("Operation", [this, &types](const XMLNode xml, void*& obj){
				auto& operater = *static_cast<Operator*>(obj);
				obj = nullptr;

				if (auto id = xml.GetAttribute("ID").GetValue()) {
					if (auto lhs = xml.GetAttribute("LHS").GetValue()) {
						Types::Type* lhs_type = types.GetType(lhs.AsString());
						Types::Type* rhs_type = nullptr;
						if (auto rhs = xml.GetAttribute("RHS").GetValue()) {
							rhs_type = types.GetType(rhs.AsString());
						}
						auto& operation = operater.AddOperation(id, lhs_type, rhs_type);
						operation.SetLHV(xml.GetAttribute("LHV")->AsNumber<int>());
						operation.SetRHV(xml.GetAttribute("RHV")->AsNumber<int>());
					}
				}
			});
		}
		Operators::Operators()
		{ }
	}
}