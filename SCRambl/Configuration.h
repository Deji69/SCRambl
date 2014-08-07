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

namespace SCRambl
{
	/*\
	 * Configuration - Configurable component
	 * AKA big ugly wrapper for SCR XML loading
	\*/
	class Configuration
	{
	private:
		std::string								m_Name;

	public:
		Configuration(std::string name) : m_Name(name)
		{ }
		virtual ~Configuration() { }

		inline const std::string & GetName() const		{ return m_Name; }

	private:
		class Object;

		class Config {
			friend class Configuration;

		protected:
			inline virtual ~Config() { }

		private:
			std::map<std::string, std::shared_ptr<Object>>	m_Objects;

			void LoadChildXML(pugi::xml_object_range<pugi::xml_node_iterator> root, std::shared_ptr<void> ptr = nullptr) {
				if (m_Objects.empty()) return;
				for (auto node : root) {
					auto it = m_Objects.find(node.name());
					if (it != m_Objects.end())
					{
						auto obj = it->second;
						ptr = obj->LoadXML(node, ptr);
						obj->LoadChildXML(node.children(), ptr);
						continue;
					}
				}
			}

		public:
			inline Config & AddClass(const std::string & name) {
				auto obj = std::make_shared<Object>();
				m_Objects.emplace(name, obj);
				return *obj;
			}
			template<typename T>
			inline Config & AddClass(const std::string & name, T& func) {
				auto obj = std::make_shared<Object>(func);
				m_Objects.emplace(name, obj);
				return *obj;
			}
		};
		class Object : public Config {
			friend class Configuration;
			std::shared_ptr<void>		m_Func;

		public:
			Object() = default;

			template<typename T>
			Object(T& func)
			{
				m_Func = std::make_shared<decltype(to_function(func))>(to_function(func));
			}

		private:
			std::shared_ptr<void> LoadXML(pugi::xml_node node, std::shared_ptr<void> theptr = nullptr)
			{
				if (m_Func) {
					auto & func = *static_cast<std::function<void(const pugi::xml_node, std::shared_ptr<void> &)>*>(m_Func.get());
					func(node, theptr);
				}
				return theptr;
			}
		};

		std::map<std::string, std::shared_ptr<Object>>			m_Objects;

	public:
		template<typename T>
		inline Config & AddClass(const std::string & name, T& func)
		{
			auto obj = std::make_shared<Object>(func);
			m_Objects.emplace(name, obj);
			return *obj;
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
	};
}