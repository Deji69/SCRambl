/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Configuration.h"
#include "SCR.h"

namespace SCRambl
{
	class Engine;

	class Constants {
	public:
		enum ConstantType {
			Null, Integer, Float, String, Enumerator
		};

		using ConstantMap = std::unordered_multimap < std::string, std::pair<ConstantType, std::shared_ptr<SCR::IConstant>> >;
		using EnumMap = std::unordered_multimap < std::string, std::shared_ptr<SCR::Enum> > ;

	private:
		Engine			&	m_Engine;
		ConstantMap			m_ConstantMap;
		EnumMap				m_EnumMap;
		std::shared_ptr<Configuration> m_Config;

	public:
		Constants(Engine & engine);

		template<typename R, typename T = R>
		std::shared_ptr < SCR::Constant<R> > AddConstant(std::string name, T value);

		template<>
		std::shared_ptr < SCR::Constant<long> > AddConstant(std::string name, long val) {
			if (name.empty()) return nullptr;
			auto ptr = std::make_shared < SCR::Constant<long> >(name, val);
			if (ptr) m_ConstantMap.emplace(name, std::make_pair(Integer, ptr));
			return ptr;
		}
		template<>
		std::shared_ptr < SCR::Constant<long> > AddConstant(std::string name, int val) {
			return AddConstant<long, long>(name, val);
		}

		SCR::Enum::Shared AddEnum(std::string name) {
			auto ptr = std::make_shared < SCR::Enum >(name);
			if (ptr) m_EnumMap.emplace(name, ptr);
			return ptr;
		}

		SCR::Enum::EnumeratorShared AddEnumerator(SCR::Enum::Shared enu, std::string name) {
			auto ptr = enu->AddEnumerator(name);
			if (ptr) m_ConstantMap.emplace(name, std::make_pair(Enumerator, ptr));
			return ptr;
		}
		SCR::Enum::EnumeratorShared AddEnumerator(SCR::Enum::Shared enu, std::string name, long val) {
			auto ptr = enu->AddEnumerator(name, val);
			if (ptr) m_ConstantMap.emplace(name, std::make_pair(Enumerator, ptr));
			return ptr;
		}
	};
}