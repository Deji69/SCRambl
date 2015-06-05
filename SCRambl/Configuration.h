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
		
		template<typename T>
		inline XMLConfig* AddClass(const std::string & name, T& func) {
			auto pr = m_Objects.emplace(name, func);
			return pr.second ? &pr.first->second : nullptr;
		}
	};

	class XMLObject : public XMLConfig
	{
		friend class Configuration;
		std::function<void(const XMLNode, void*&)> m_Func;

	public:
		XMLObject() = default;
		template<typename T>
		XMLObject(T& func) {
			m_Func = func;
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

	/*\
	 * Configuration - Configurable component
	 * AKA big ugly wrapper for SCR XML loading
	\*/
	/*class Configuration
	{
	private:
		std::string								m_Name;

	public:
		Configuration(std::string name) : m_Name(name)
		{ }
		virtual ~Configuration() { }

		inline const std::string & GetName() const		{ return m_Name; }

	private:

	public:
		class Object;
		class Config {
			friend class Configuration;

		protected:
			inline virtual ~Config() { }

		private:
			std::map<std::string, Object> m_Objects;

			void LoadChildXML(pugi::xml_object_range<pugi::xml_node_iterator> root, void* ptr = nullptr) {
				if (m_Objects.empty()) return;
				for (auto node : root) {
					auto it = m_Objects.find(node.name());
					if (it != m_Objects.end())
					{
						auto& obj = it->second;
						auto new_ptr = obj.LoadXML(node, ptr);
						obj.LoadChildXML(node.children(), new_ptr);
						continue;
					}
				}
			}

		public:
			inline Config* AddClass(const std::string & name) {
				auto pr = m_Objects.emplace(name, Object());
				return pr.second ? &pr.first->second : nullptr;
			}
			template<typename T>
			inline Config & AddClass(const std::string & name, T& func) {
				auto pr = m_Objects.emplace(name, func);
				return pr.second ? &pr.first->second : nullptr;
			}
		};
		class Object : public Config {
			friend class Configuration;
			std::unique_ptr<void> m_Func;

		public:
			Object() = default;

			template<typename T>
			Object(T& func)
			{
				m_Func = std::make_unique<decltype(to_function(func))>(to_function(func));
			}

		private:
			void* LoadXML(XMLNode node, void* theptr = nullptr)
			{
				if (m_Func) {
					auto & func = *static_cast<std::function<void(const XMLNode, void* &)>*>(m_Func.get());
					func(node, theptr);
				}
				return theptr;
			}
		};

		std::map<std::string, Object*> m_Objects;

	public:
		template<typename T>
		inline Config* AddClass(const std::string & name, T& func) {
			auto pr = m_Objects.emplace(name, func);
			return pr.second ? &pr.first->second : nullptr;
		}
		
		int LoadXML(pugi::xml_node master_node)
		{
			for (auto node : master_node.children())
			{
				if (!m_Objects.empty())
				{
					auto it = m_Objects.find(node.name());
					if (it != m_Objects.end())
					{
						auto obj = it->second;
						auto ptr = obj->LoadXML(node);
						obj->LoadChildXML(node.children(), ptr);
						continue;
					}
				}
			}
			return 0;
		}
	};*/
}