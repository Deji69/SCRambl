/**********************************************************/
// SCR Library
// Created by Deji <the_zone@hotmail.co.uk>
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
/**********************************************************/
#pragma once
#include <string>
#include <memory>
#include <unordered_map>

namespace SCR
{
	/*\
	 * SCR::IConstant - SCR type base
	\*/
	class IConstant
	{
	public:
		inline virtual ~IConstant() { };
	};

	/*\
	 * SCR::Constant - SCR constant
	\*/
	template<typename T>
	class Constant : public IConstant
	{
		unsigned long long		m_ID;
		std::string				m_Name;
		T						m_Value;

	public:
		Constant(std::string name, T val) : m_Name(name), m_Value(val)
		{
			std::hash<std::string> hasher;
			m_ID = hasher(name);
		}
		inline virtual ~Constant() { }

		inline std::string		GetName() const			{ return m_Name; }
		inline const T &		GetValue() const		{ return m_Value; }
	};

	/*\
	 * SCR::Enumerator - SCR Enumerator
	\*/
	class Enumerator : public Constant < long >
	{
	public:
		using Shared = std::shared_ptr < Enumerator > ;
		Enumerator(std::string name, long val) : Constant(name, val)
		{ }
	};

	/*\
	 * SCR::Enum - SCR Enum
	\*/
	class Enum
	{
	public:
		using Shared = std::shared_ptr < Enum > ;
		using EnumeratorShared = std::shared_ptr < Enumerator > ;
		using Map = std::unordered_map < std::string, EnumeratorShared >;

	private:
		unsigned long long		m_ID;
		long					m_Count = 0;
		std::string				m_Name;
		Map						m_Map;

	public:
		Enum(std::string name) : m_Name(name)
		{ }
		inline virtual ~Enum() { }

		inline unsigned long long GetID() const				{ return m_ID; }
		inline const std::string & GetName() const			{ return m_Name; }
		inline EnumeratorShared AddEnumerator(std::string name) {
			auto ptr = std::make_shared<Enumerator>(name, m_Count++);
			if(ptr) m_Map[name] = ptr;
			return ptr;
		}
		inline EnumeratorShared AddEnumerator(std::string name, long val) {
			if (val > m_Count) m_Count = val;
			auto ptr = std::make_shared<Enumerator>(name, val);
			if(ptr) m_Map[name] = ptr;
			return ptr;
		}
		inline EnumeratorShared FindEnumerator(std::string name) const {
			auto it = m_Map.find(name);
			if (it != m_Map.end()) {
				return it->second;
			}
			return nullptr;
		}
	};
}