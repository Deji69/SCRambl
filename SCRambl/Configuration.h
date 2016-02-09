/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include <fstream>
#include <array>
#include <set>
#include "utils.h"
#include "XML.h"

namespace SCRambl
{
	class Engine;
	class Build;
	class Builder;
	class BuildEnvironment;

	class XMLObject;

	class XMLConfig
	{
		friend class XMLConfiguration;

	protected:
		inline virtual ~XMLConfig() { }

	private:
		std::map<std::string, XMLObject> m_Objects;

		void LoadChildXML(XMLRange root, void* ptr = nullptr);

	public:
		XMLConfig() { }

		XMLConfig* AddClass(const std::string&);
		
		template<typename T, typename T2 = std::function<void(const XMLNode, void*&)>>
		inline XMLConfig* AddClass(const std::string& name, T& func, T2& recurse_func = T2()) {
			auto pr = m_Objects.emplace(name, XMLObject(func, recurse_func));
			return pr.second ? &pr.first->second : nullptr;
		}
	};

	class XMLObject : public XMLConfig
	{
		friend class Configuration;
		std::function<void(const XMLNode, void*&)> m_Func;
		std::function<void(const XMLNode, void*&)> m_RecurseFunc;

	public:
		XMLObject() = default;
		template<typename T, typename T2 = std::function<void(const XMLNode, void*&)>>
		XMLObject(T& func, T2& recurse_func = T2()) {
			m_Func = func;
			m_RecurseFunc = recurse_func;
		}
		~XMLObject() { }

		void* LoadXML(XMLNode node, void* theptr = nullptr);
	};

	class XMLConfiguration
	{
	private:
		std::string m_Name;
		std::map<std::string, XMLObject> m_Objects;

	public:
		XMLConfiguration(std::string name);
		
		template<typename T>
		XMLConfig* AddClass(const std::string& name, T& func) {
			auto pr = m_Objects.emplace(name, func);
			return pr.second ? &pr.first->second : nullptr;
		}

		void LoadXML(XMLNode main_node);
	};
}