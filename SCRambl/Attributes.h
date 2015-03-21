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
#include "SCR.h"

namespace SCRambl
{
	class Engine;

	template<typename TID, typename T>
	class Attribute {
		TID			m_ID;
		T			m_Val;

	public:
		inline const TID &	GetID() const			{ return m_ID; }
		inline T &			GetValue()				{ return m_Val; }
		inline const T &	GetValue() const		{ return m_Val; }

	private:
	};

	template<typename T, typename TKey = std::string, typename TLess = std::less<TKey>>
	class AttributeSet {
		std::map<TKey, T, TLess>		m_Map;
		const T							m_Default;

	public:
		AttributeSet(T default_id) : m_Default(default_id)
		{ }
		AttributeSet(T default_id, const std::map<TKey, T, TLess> & set) : m_Default(default_id), m_Map(map)
		{ }

		inline T GetAttribute(TKey key) const {
			auto it = m_Map.find(key);
			return it != m_Map.end() ? it->second : m_Default;
		}

	protected:
		inline void AddAttribute(TKey key, T attr) {
			m_Map.emplace(key, attr);
		}
	};

	/*\
	 * Attributes - Standard dynamic handling of object attributes
	 * TSet = AttributeSet or derivitive class
	\*/
	template<typename TSet>
	class Attributes {
		TSet			m_Set;
	};
}