/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include "utils.h"
#include "Configuration.h"
#include "SCR.h"

namespace SCRambl
{
	class Engine;

	/*\
	 * Types - SCR type manager
	\*/
	class Types
	{
	public:
		typedef std::unordered_map < std::string, SCR::Type::Shared > Map;

	private:
		Engine					&	m_Engine;
		Configuration::Shared		m_Config;
		Map							m_Map;

	public:
		Types(Engine & eng);

		inline SCR::Type::Shared AddType(std::string name, unsigned long long id)
		{
			auto type = std::make_shared<SCR::Type>(id, name);
			m_Map.emplace(name, type);
			return type;
		}
		inline SCR::Type::Shared AddExtendedType(std::string name, unsigned long long id, SCR::Type::CShared type = nullptr)
		{
			auto full_type = SCR::ExtendedType::MakeShared(id, name, type);
			m_Map.emplace(name, full_type);
			return full_type;
		}
		template<typename T>
		inline SCR::Type::Shared AddVariableType(std::string name, unsigned long long id, T var_type, SCR::Type::CShared type = nullptr)
		{
			auto vartype = SCR::VarType<T>::MakeShared(id, name, var_type, type);
			m_Map.emplace(name, vartype);
			return vartype;
		}
		inline SCR::Type::Shared GetType(const std::string & name) const
		{
			if (m_Map.empty()) return nullptr;
			auto it = m_Map.find(name);
			if (m_Map.end() == it) return nullptr;
			return it->second;
		}
	};
}