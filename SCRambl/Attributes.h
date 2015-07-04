/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include <set>
#include "utils.h"
#include "Configuration.h"
#include "XML.h"
#include "SCR.h"

namespace SCRambl
{
	class Engine;

	template<typename T, typename TKey = std::string, typename TLess = std::less<TKey>>
	class AttributeSet {
	public:
		using Map = std::map<TKey, T, TLess>;

		AttributeSet(T default_id) : m_Default(default_id)
		{ }
		AttributeSet(T default_id, const std::map<TKey, T, TLess>& map) : m_Default(default_id), m_Map(map)
		{ }

		inline T GetAttribute(TKey key) const {
			auto it = m_Map.find(key);
			return it != m_Map.end() ? it->second : m_Default;
		}
		inline const Map& GetMap() const { return m_Map; }

		inline size_t Size() const { return m_Map.size(); }
		inline typename Map::const_iterator begin() const { return m_Map.begin(); }
		inline typename Map::const_iterator end() const { return m_Map.end(); }

	protected:
		inline void AddAttribute(TKey key, T attr_id) {
			m_Map.emplace(key, attr_id);
		}
	
	private:
		Map m_Map;
		const T m_Default;
	};

	/*\
	 * Attributes - Fondling of object attributes
	\*/
	template<typename TAttributes, TAttributes TMax>
	class Attributes {
	public:
		using Set = AttributeSet<TAttributes>;

	public:
		Attributes(const Set& set) : m_Set(set)
		{ }
		virtual ~Attributes() { }

		TAttributes GetAttributeID(std::string name) const {
			return m_Set.GetAttribute(name);
		}
		void SetAttribute(TAttributes id, XMLValue value) {
			m_Attributes[id] = value;
		}
		void SetAttribute(std::string id, XMLValue value) {
			SetAttribute(GetAttributeID(id), value);
		}
		XMLValue GetAttribute(TAttributes id) const {
			auto it = m_Attributes.find(id);
			return it != m_Attributes.end() ? it->second : "";
		}
		XMLValue GetAttribute(std::string id) const {
			return GetAttribute(GetAttributeID(id));
		}

	private:
		const Set& m_Set;
		std::map<TAttributes, XMLValue> m_Attributes;
	};
}