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
	class Build;

	class Constants {
	public:
		enum ConstantType {
			Null, Integer, Float, String, Enumerator
		};

		using ConstantMap = std::unordered_multimap<std::string, std::pair<ConstantType, SCR::IConstant*>>;
		using EnumMap = std::unordered_multimap<std::string, SCR::Enum* > ;

	private:
		ConstantMap m_ConstantMap;
		EnumMap	m_EnumMap;
		XMLConfiguration* m_Config;

	public:
		Constants();

		void Init(Build&);

		template<typename R, typename T = R>
		SCR::Constant<R>* AddConstant(std::string name, T value);

		template<>
		SCR::Constant<long>* AddConstant(std::string name, long val) {
			if (name.empty()) return nullptr;
			auto ptr = new SCR::Constant<long>(name, val);
			if (ptr) m_ConstantMap.emplace(name, std::make_pair(Integer, ptr));
			return ptr;
		}
		template<>
		SCR::Constant<long>* AddConstant(std::string name, int val) {
			return AddConstant<long, long>(name, val);
		}

		SCR::Enum* AddEnum(std::string name) {
			auto ptr = new SCR::Enum(name);
			if (ptr) m_EnumMap.emplace(name, ptr);
			return ptr;
		}

		SCR::Enumerator* AddEnumerator(SCR::Enum* enu, std::string name) {
			auto ptr = enu->AddEnumerator(name);
			if (ptr) m_ConstantMap.emplace(name, std::make_pair(Enumerator, ptr));
			return ptr;
		}
		SCR::Enumerator*AddEnumerator(SCR::Enum* enu, std::string name, long val) {
			auto ptr = enu->AddEnumerator(name, val);
			if (ptr) m_ConstantMap.emplace(name, std::make_pair(Enumerator, ptr));
			return ptr;
		}
	};
}