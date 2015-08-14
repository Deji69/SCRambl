#include "stdafx.h"
#include "utils.h"
#include "Operators.h"
#include "Builder.h"
#include "Variables.h"

using namespace SCRambl;
using namespace SCRambl::Operators;

Operator::Sign Operator::GetSign(std::string str) {
	static const std::map<std::string, Sign, i_less> map = {
		{ "negative", Sign::negative },
		{ "positive", Sign::positive }
	};
	auto it = map.find(str);
	return it != map.end() ? it->second : Sign::none;
}

/* Operators::Operator */
OperationRef Operator::GetOperation(Variable* var, const Types::Type* type) {
	Operation* bestMatch = nullptr;
	Types::MatchLevel bestMatchLevel = Types::MatchLevel::None;
	std::vector<Operation*> basicMatches, looseMatches;
	for (auto& op : m_Operations) {
		if (!op.HasLHS() || !op.HasRHS()) continue;
				
		auto lhs = op.GetLHS(), rhs = op.GetRHS();
		auto lhs_lvl = lhs->GetMatchLevel(var->Type());
		auto rhs_lvl = rhs->GetMatchLevel(type);
		if (lhs_lvl != rhs_lvl) {
			if (lhs_lvl == Types::MatchLevel::None || rhs_lvl == Types::MatchLevel::None)
				lhs_lvl = rhs_lvl = Types::MatchLevel::None;
			else if (lhs_lvl == Types::MatchLevel::Loose || rhs_lvl == Types::MatchLevel::Loose)
				lhs_lvl = rhs_lvl = Types::MatchLevel::Loose;
			else if (lhs_lvl == Types::MatchLevel::Basic || rhs_lvl == Types::MatchLevel::Basic)
				lhs_lvl = rhs_lvl = Types::MatchLevel::Basic;
		}

		// We want the best of the best of the best
		if (lhs_lvl == Types::MatchLevel::None)
			continue;
		if (bestMatch) {
			if (lhs_lvl == bestMatchLevel)
				continue;
			if (lhs_lvl == Types::MatchLevel::Loose && bestMatchLevel != Types::MatchLevel::Loose)
				continue;
			if (lhs_lvl == Types::MatchLevel::Basic && bestMatchLevel == Types::MatchLevel::Strict)
				continue;
		}

		bestMatch = &op;
		bestMatchLevel = lhs_lvl;
	}
	return bestMatch ? OperationRef() : bestMatch->GetRef();
}
OperationRef Operator::GetUnaryOperation(Variable* var, bool rhs_var) {
	std::vector<Operation*> basicMatches, looseMatches;
	for (auto& op : m_Operations) {
		if (op.HasLHS() == op.HasRHS()) continue;
		if (rhs_var ? (!op.HasLHV() || !op.HasRHS()) : (!op.HasRHV() || !op.HasLHS())) continue;

		auto lhs = op.GetLHS(), rhs = op.GetRHS();

		if (!lhs && !rhs) continue; // derp?

		auto matchLevel = var->Type()->GetMatchLevel(rhs_var ? rhs : lhs);
		if (matchLevel == Types::MatchLevel::Strict)
			return op.GetRef();
		else if (matchLevel == Types::MatchLevel::Basic)
			basicMatches.emplace_back(&op);
		else if (matchLevel == Types::MatchLevel::Loose && basicMatches.empty())
			looseMatches.emplace_back(&op);
	}
	if (!basicMatches.empty()) {
		return basicMatches.front()->GetRef();
	}
	else if (!looseMatches.empty()) {
		return looseMatches.front()->GetRef();
	}
	return OperationRef();
}
/* Operators::Master */
void Master::Init(Build& build) {
	auto& types = build.GetTypes();

	m_Config = build.AddConfig("Operators");

	auto operater = m_Config->AddClass("Operator", [this](const XMLNode xml, void*& obj){
		// add operator
		auto name = xml["Name"];
		if (name) {
			auto ass_attr = xml["Assign"];	// teeheehee
			auto sign_attr = xml["Sign"];
			auto cond_attr = xml["Cond"];
			auto op = Add(name->AsString(), Operator::GetSign(sign_attr->AsString()), cond_attr->AsBool(), ass_attr->AsBool());
			if (op) {
				// map pseudonyms
				auto comp_attr = xml["Comp"];
				auto not_attr = xml["Not"];
				if (*comp_attr) Add(comp_attr->AsString(), op, OperatorType::Compound);
				if (*not_attr) Add(not_attr->AsString(), op, OperatorType::Not);
			}
			obj = op.Ptr();
		}
		else obj = nullptr;
	});
	operater->AddClass("Operation", [this, &types](const XMLNode xml, void*& obj){
		auto& operater = *static_cast<Operator*>(obj);
		obj = nullptr;

		if (auto id = xml.GetAttribute("ID").GetValue()) {
			Types::Type * lhs_type = nullptr, * rhs_type = nullptr;
			if (auto lhs = xml.GetAttribute("LHS").GetValue())
				lhs_type = types.GetType(lhs.AsString());
			if (auto rhs = xml.GetAttribute("RHS").GetValue())
				rhs_type = types.GetType(rhs.AsString());
					
			if (lhs_type || rhs_type) {
				auto operation = operater.AddOperation(id.AsNumber<size_t>(), lhs_type, rhs_type);
				if (auto lhv = xml.GetAttribute("LHV"))
					operation->SetLHV(lhv->AsNumber<int>());
				if (auto rhv = xml.GetAttribute("RHV"))
					operation->SetRHV(rhv->AsNumber<int>());
				operation->SetSwapped(xml.GetAttribute("Swap")->AsBool(false));
			}
		}
	});
}
Master::Master()
{ }