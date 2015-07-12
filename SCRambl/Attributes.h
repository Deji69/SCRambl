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
			auto it = m_Map.find(tolower(key));
			return it != m_Map.end() ? it->second : m_Default;
		}
		inline const Map& GetMap() const { return m_Map; }

		inline size_t Size() const { return m_Map.size(); }
		inline typename Map::const_iterator begin() const { return m_Map.begin(); }
		inline typename Map::const_iterator end() const { return m_Map.end(); }

	protected:
		inline void AddAttribute(TKey key, T attr_id) {
			m_Map.emplace(tolower(key), attr_id);
		}
	
	private:
		Map m_Map;
		const T m_Default;
	};

	/*\
	 * IAttributes
	\*/
	class IAttributes {
	public:
		virtual ~IAttributes() { }

		virtual void SetAttribute(const char* attr, XMLValue val) = 0;
		virtual XMLValue GetAttribute(const char* attr) const = 0;
	};

	/*\
	 * Attributes - Fondling of object attributes
	\*/
	template<typename TAttributes, typename TAttributeSet = AttributeSet<TAttributes>>
	class Attributes : public IAttributes {
		static const TAttributeSet s_AttributeSet;

	public:
		using Set = AttributeSet<TAttributes>;

	public:
		Attributes()
		{ }
		virtual ~Attributes() { }

		TAttributes GetAttributeID(std::string name) const {
			return s_AttributeSet.GetAttribute(name);
		}
		void SetAttribute(TAttributes id, XMLValue value) {
			m_Attributes[id] = value;
		}
		void SetAttribute(std::string id, XMLValue value) {
			SetAttribute(GetAttributeID(id), value);
		}
		void SetAttribute(const char* id, XMLValue value) override {
			SetAttribute(GetAttributeID(id), value);
		}
		XMLValue GetAttribute(TAttributes id) const {
			auto it = m_Attributes.find(id);
			return it != m_Attributes.end() ? it->second : "";
		}
		XMLValue GetAttribute(std::string id) const {
			return GetAttribute(GetAttributeID(id));
		}
		XMLValue GetAttribute(const char* id) const override {
			return GetAttribute(GetAttributeID(id));
		}

	private:
		std::map<TAttributes, XMLValue> m_Attributes;
	};
}