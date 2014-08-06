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
	\*/
	class Configuration
	{
	protected:
		enum class ConfigType { list, object, setting };

	private:
		std::string								m_Name;
		ConfigType								m_Type;

	public:
		Configuration(std::string name) : m_Name(name)
		{ }
		virtual ~Configuration() { }

		inline const std::string & GetName() const		{ return m_Name; }

	private:
		template<typename T, typename... TStr>
		static inline std::array<T, sizeof...(TStr)> StraightenPack(TStr&&... attributes) {
			return{ std::forward<TStr>(attributes)... };
		}
		class Object;
		class Config {
			friend class Configuration;
		public:
			enum class Type { list, object, setting };

		protected:
			inline virtual ~Config() { }
			inline Type GetType() const { return m_Type; }

		private:
			Type							m_Type;
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
			Config(Type type) : m_Type(type)
			{ }

			inline Config & AddClass(const std::string & name) {
				auto obj = std::make_shared<Object>();
				m_Objects.emplace(name, obj);
				return *obj;
			}
			template<typename T, typename... TStr>
			inline Config & AddClass(const std::string & name, T& func, TStr&&...arr) {
				auto obj = std::make_shared<Object>(func, StraightenPack<std::string>(arr...));
				m_Objects.emplace(name, obj);
				return *obj;
			}
		};
		class Object : public Config {
			friend class Configuration;
			std::vector<std::string>	m_Attributes;
			std::shared_ptr<void>		m_Func;

		public:
			Object() : Config(Type::list) {
			}
			template<typename T>
			Object(T& func) : Config(Type::list) {
				m_Func = std::make_shared<decltype(to_function(func))>(to_function(func));
			}
			template<typename T, typename T2>
			Object(T& func, T2&& attributes) : Config(Type::object) {
				for (auto & attr : attributes) {
					m_Attributes.emplace_back(attr);
				}

				m_Func = std::make_shared<decltype(to_function(func))>(to_function(func));
			}

		private:
			/*inline void operator()() {
				ASSERT(GetType() == Type::list);
				if (!m_Func) return;
				auto & func = *static_cast<std::function<void()>*>(m_Func.get());
				func();
			}
			inline void operator()(const std::vector<pugi::xml_attribute> & vec) {
				ASSERT(GetType() == Type::object);
				if (!m_Func) return;
				auto & func = *static_cast<std::function<void(const std::vector<pugi::xml_attribute>, std::shared_ptr<void> &)>*>(m_Func.get());
				std::shared_ptr<void> ptr;
				func(vec, ptr);
			}*/
			std::shared_ptr<void> LoadXML(pugi::xml_node node, std::shared_ptr<void> theptr = nullptr) {
				if (m_Func) {
					std::vector<pugi::xml_attribute> attributes;
					for (auto & attr : m_Attributes) {
						attributes.push_back(node.attribute(attr.c_str()));
					}
					auto & func = *static_cast<std::function<void(const std::vector<pugi::xml_attribute>, std::shared_ptr<void> &)>*>(m_Func.get());
					func(attributes, theptr);
				}
				return theptr;
			}
		};

		std::map<std::string, std::shared_ptr<Object>>			m_Objects;

	public:
		template<typename T, typename... TStr>
		inline Config & AddClass(const std::string & name, T& func, TStr&&...arr)
		{
			auto obj = std::make_shared<Object>(func, StraightenPack<std::string>(arr...));
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