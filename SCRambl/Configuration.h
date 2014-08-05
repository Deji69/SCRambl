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
//#include "Tasks.h"
//#include "Scripts.h"

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
		/*template<typename... TStr>
		inline void SortAttributes(TStr&&... attributes)
		{
			auto tuple = std::forward_as_tuple(attributes...);

		}*/
		class Config {
		protected:
			virtual ~Config() { }
		};
		class Object : Config {
			friend class Configuration;
			std::vector<std::string>	m_Attributes;
			std::shared_ptr<void>		m_Func;

		public:
			template<typename T, typename T2>
			Object(T& func, T2&& attributes)
			{
				for (auto & attr : attributes) {
					m_Attributes.emplace_back(attr);
				}

				m_Func = std::make_shared<decltype(to_function(func))>(to_function(func));
			}

			inline void operator()(const std::vector<pugi::xml_attribute> & vec) {
				auto & func = *static_cast<std::function<void(const std::vector<pugi::xml_attribute>)>*>(m_Func.get());
				func(vec);
			}
		};

		std::map<std::string, Object>			m_Objects;

	public:
		template<typename T, typename... TStr>
		inline void AddClass(const std::string & name, T& func, TStr&&...arr)
		{
			m_Objects.emplace(name, Object(func, StraightenPack<std::string>(arr...)));
		}
		
		int LoadXML(pugi::xml_node master_node)
		{
			for (auto node : master_node.children())
			{
				if (m_Objects.size())
				{
					auto it = m_Objects.find(node.name());
					if (it != m_Objects.end())
					{
						// get the object attributes
						std::vector<pugi::xml_attribute> attributes;
						auto & obj = it->second;
						for (auto & attr : obj.m_Attributes)
						{
							attributes.push_back(node.attribute(attr.c_str()));
						}

						obj(attributes);
						continue;
					}
				}
			}
			return 0;
		}
	};
}