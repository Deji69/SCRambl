/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <memory>
#include "Types.h"

namespace SCRambl
{
	class Variable
	{
		std::string	m_Name;
		Types::Type* m_Type;
		Types::VarType m_Value;
		size_t m_ArraySize = 0;
		bool m_IsGlobal = false;

		void Init()
		{ }

	public:
		Variable(Types::Type* type, std::string name, size_t array_size = 0) : m_Name(name), m_Type(type), m_ArraySize(array_size),
			m_Value(array_size == 0 ? *m_Type->GetVarType() : *m_Type->GetArrayType())
		{ Init(); }

		inline std::string Name() const { return m_Name; }
		inline Types::Type* Type() const { return m_Type; }
		inline Types::VarType Value() const { return m_Value; }
		inline bool IsArray() const { return m_ArraySize != 0; }
		inline bool IsGlobal() const { return m_Value.GetType()->IsGlobal(); }

		Types::Value* GetValue() const {
			Types::Value* best_value = nullptr;
			m_Type->Values<Types::VariableValue>(IsArray() ? Types::ValueSet::Array : Types::ValueSet::Variable, [&best_value](Types::VariableValue* value){
				best_value = value;
				return true;
			});
			return best_value;
		}
	};
}