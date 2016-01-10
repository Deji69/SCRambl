#include "stdafx.h"
#include "utils.h"
#include "Operators.h"
#include "Builder.h"
#include "Variables.h"
#include "Operands.h"

using namespace SCRambl;
using namespace SCRambl::Operators;

/* Operators::Operation */
Operand Operation::EvaluateAuto(const Operand& lop, const Operand& rop) {
	ASSERT(m_AutoType != max_operator);
	auto op = lop;
	auto& pr_op = lop;
	int64_t intVal;
	double fltVal;
	switch (pr_op.GetType()) {
	case Operand::Type::NullValue:
	case Operand::Type::IntValue:
	case Operand::Type::LabelValue:
	case Operand::Type::VariableValue:
		if (pr_op.IsLabelType())
			intVal = op.Value<ScriptLabel>()->Offset();
		else if (pr_op.IsVariableType())
			intVal = op.Value<ScriptVariable>()->Index();
		else
			intVal = op.Value<int64_t>();

		if (rop.IsNumberType())
			intVal = AutoResult(m_AutoType, intVal, rop.GetType() == Operand::IntValue ? rop.Value<int64_t>() : rop.Value<double>());
		else if (rop.IsLabelType())
			intVal = AutoResult(m_AutoType, intVal, rop.Value<ScriptLabel>()->Offset());
		else if (rop.IsVariableType())
			intVal = AutoResult(m_AutoType, intVal, rop.Value<ScriptVariable>()->Index());
		return Operand(intVal, std::to_string(intVal));

	case Operand::Type::FloatValue:
		if (rop.IsNumberType())
			fltVal = AutoResult(m_AutoType, op.Value<double>(), rop.GetType() == Operand::IntValue ? rop.Value<int64_t>() : rop.Value<double>());
		else if (rop.IsLabelType())
			fltVal = AutoResult(m_AutoType, op.Value<double>(), rop.Value<ScriptLabel>()->Offset());
		else if (rop.IsVariableType())
			fltVal = AutoResult(m_AutoType, op.Value<double>(), rop.Value<ScriptVariable>()->Index());
		return Operand(fltVal, std::to_string(fltVal));
	}
	return op;
}
Operation::Attributes Operation::GetAttributes() const {
	Attributes attr;
	attr.SetAttribute(Types::DataAttributeID::ID, m_Index);
	attr.SetAttribute(Types::DataAttributeID::Name, m_Operator->Name());
	attr.SetAttribute(Types::DataAttributeID::NumArgs, (m_HasLHV ? 1 : 0) + (m_HasRHV ? 1 : 0));
	return attr;
}

/* Operators::OperationValue */
size_t OperationValue::GetValueSize(const Operation::Attributes& op) const {
	switch (GetDataType()) {
	case Types::DataType::Int:
		return CountBitOccupation(op.GetAttribute(GetAttributeID()).AsNumber<long long>());
	case Types::DataType::String:
		return op.GetAttribute(GetAttributeID()).AsString().size() * 8;
	case Types::DataType::Float:
		return sizeof(float);
	case Types::DataType::Char:
		return sizeof(char);
	default: BREAK();
	}
	return 0;
}

/* Operators::Operator */
Operator::Sign Operator::GetSign(std::string str) {
	static const std::map<std::string, Sign, i_less> map = {
		{ "negative", Sign::negative },
		{ "positive", Sign::positive }
	};
	auto it = map.find(str);
	return it != map.end() ? it->second : Sign::none;
}
OperationRef Operator::GetAutoOperation(const Types::Type* ltype, const Types::Type* rtype) {
	Operation* bestMatch = nullptr;
	Types::MatchLevel bestMatchLevel = Types::MatchLevel::None;
	std::vector<Operation*> basicMatches, looseMatches;
	for (auto& op : m_Autos) {
		if (!op.HasLHS() || !op.HasRHS()) continue;

		auto lhs = op.LHS(), rhs = op.RHS();
		auto lhs_lvl = lhs->GetMatchLevel(ltype);
		auto rhs_lvl = rhs->GetMatchLevel(rtype);
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
	return bestMatch ? bestMatch->GetRef() : OperationRef();
}
OperationRef Operator::GetOperation(Variable* var, const Types::Type* type) {
	Operation* bestMatch = nullptr;
	Types::MatchLevel bestMatchLevel = Types::MatchLevel::None;
	std::vector<Operation*> basicMatches, looseMatches;
	for (auto& op : m_Operations) {
		if (!op.HasLHS() || !op.HasRHS()) continue;
				
		auto lhs = op.LHS(), rhs = op.RHS();
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
	return bestMatch ? bestMatch->GetRef() : OperationRef();
}
OperationRef Operator::GetUnaryOperation(Variable* var, bool rhs_var) {
	std::vector<Operation*> basicMatches, looseMatches;
	for (auto& op : m_Operations) {
		if (op.HasLHS() == op.HasRHS()) continue;
		if (rhs_var ? (!op.HasLHV() || !op.HasRHS()) : (!op.HasRHV() || !op.HasLHS())) continue;

		auto lhs = op.LHS(), rhs = op.RHS();

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
Type Master::GetTypeByName(std::string name) {
	static std::map<std::string, Type, i_less> map = {
		{ "add", Type::add }, { "sub", Type::sub }, { "mul", Type::mult }, { "div", Type::div }, { "mod", Type::mod },
		{ "inc", Type::inc }, { "dec", Type::dec },

		{ "bit_and", Type::bit_and }, { "bit_or", Type::bit_or }, { "bit_xor", Type::bit_xor },
		{ "bit_shl", Type::bit_shl }, { "bit_shr", Type::bit_shr }, { "bit_not", Type::bit_not },

		{ "comp_add", Type::comp_add }, { "comp_sub", Type::comp_sub }, { "comp_mul", Type::comp_mult }, { "comp_div", Type::comp_div },
		{ "comp_mod", Type::comp_mod },
		{ "comp_bit_and", Type::comp_bit_and }, { "comp_bit_or", Type::comp_bit_or }, { "comp_bit_xor", Type::comp_bit_xor },
		{ "comp_bit_shl", Type::comp_bit_shl }, { "comp_bit_shr", Type::comp_bit_shr },
		{ "uncomp_cast", Type::uncomp_cast },

		{ "eq", Type::eq }, { "neq", Type::neq }, { "gt", Type::gt }, { "lt", Type::lt }, { "gte", Type::geq }, { "lte", Type::leq },
		{ "not", Type::not }, { "and", Type::and }, { "or", Type::or }, { "if", Type::cond }, { "el", Type::condel },
	};
	auto it = map.find(name);
	return it != map.end() ? it->second : Type::max_operator;
}
void Master::Init(Build& build) {
	auto& types = build.GetTypes();
	VecRef<Types::Type> type;

	m_Config = build.AddConfig("Operators");

	m_Config->AddClass("OperatorType", [this, &type, &types](const XMLNode xml, void*& obj){
		type = types.GetType(xml["Type"]->AsString()).Ref();
		if (!type) BREAK();
	});
	auto operater = m_Config->AddClass("Operator", [this, &type](const XMLNode xml, void*& obj){
		// add operator
		auto name = xml["Name"];
		if (name) {
			auto ass_attr = xml["Assign"];	// teeheehee
			auto sign_attr = xml["Sign"];
			auto cond_attr = xml["Cond"];
			auto op = Add(name->AsString(), type, Operator::GetSign(sign_attr->AsString()), cond_attr->AsBool(), ass_attr->AsBool());
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
	
	auto func_addOperation = [this, &types](const XMLNode xml, Operator& op, bool isAuto){
		auto id = xml.GetAttribute("ID").GetValue();
		if (id || isAuto) {
			VecRef<Types::Type> lhs_type, rhs_type;
			if (auto lhs = xml.GetAttribute("LHS").GetValue())
				lhs_type = types.GetType(lhs.AsString()).Ref();
			if (auto rhs = xml.GetAttribute("RHS").GetValue())
				rhs_type = types.GetType(rhs.AsString()).Ref();

			if (lhs_type || rhs_type) {
				auto operation = isAuto ? op.AddAuto(lhs_type, rhs_type, id.AsNumber<size_t>()) : op.AddOperation(id.AsNumber<size_t>(), lhs_type, rhs_type);
				if (auto auto_operation = xml.GetAttribute("Operation"))
					operation->SetAutoType(GetTypeByName(auto_operation->AsString()));
				if (auto lhv = xml.GetAttribute("LHV"))
					operation->SetLHV(lhv->AsNumber<int>());
				if (auto rhv = xml.GetAttribute("RHV"))
					operation->SetRHV(rhv->AsNumber<int>());
				operation->SetSwapped(xml.GetAttribute("Swap")->AsBool(false));
			}
		}
	};

	operater->AddClass("Operation", [this, &types, func_addOperation](const XMLNode xml, void*& obj){
		auto& operater = *static_cast<Operator*>(obj);
		obj = nullptr;
		return func_addOperation(xml, operater, false);
	});
	operater->AddClass("Auto", [this, &types, func_addOperation](const XMLNode xml, void*& obj){
		auto& operater = *static_cast<Operator*>(obj);
		obj = nullptr;
		return func_addOperation(xml, operater, true);
	});
}
Master::Master()
{ }