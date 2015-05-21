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
#include "Types.h"

namespace SCR
{
	/*\
	 * Command Arg - SCR Command Arg
	\*/
	template<typename TBasicType = Default::BasicType>
	class CommandArg
	{
		const TBasicType			&	m_Type;
		int								m_Index;				// nth arg
		bool							m_IsReturn = false;
		//std::string					m_Description;

	public:
		typedef TBasicType BasicType;
		using Shared = std::shared_ptr < CommandArg >;
		using CShared = std::shared_ptr < const CommandArg >;
		typedef typename std::template vector < CommandArg > Vector;
		typedef typename Vector::iterator Iterator;
		/*using Iterator = Vector::iterator;
		using CIterator = Vector::const_iterator;
		using RIterator = Vector::reverse_iterator;
		using CRIterator = Vector::const_reverse_iterator;*/

		CommandArg(const TBasicType & type, int index, bool isRet = false) : m_Type(type), m_Index(index), m_IsReturn(isRet)
		{ }

		inline bool			IsReturn() const			{ return m_IsReturn; }
		inline int			GetIndex() const			{ return m_Index; }
		inline const TBasicType & GetType() const		{ return m_Type; }
	};

	namespace Default {
		typedef SCR::CommandArg<Default::BasicType> CommandArg;
	}

	/*\
	 * Command - SCR command
	\*/
	template<typename TArgType = Default::CommandArg>
	class Command
	{
	public:
		typedef TArgType Arg;
		typedef std::shared_ptr< Command<TArgType> > Shared;
		typedef std::vector< TArgType > ArgList;

	private:
		//uint64_t				m_Index;
		unsigned long long		m_Index;				// unique index/hash
		std::string				m_Name;					// command name/id
		ArgList					m_Args;

	public:
		Command(std::string name, unsigned long long index) : m_Name(name), m_Index(index)
		{ }

		void AddArg(const typename Arg::BasicType & type, bool isRet = false) {
			m_Args.emplace_back(type, m_Args.size(), isRet);
		}

		inline TArgType& GetArg(size_t i) { return m_Args[i]; }
		inline const TArgType& GetArg(size_t i) const { return m_Args[i]; }

		inline typename ArgList::const_iterator	BeginArg() const		{ return m_Args.begin(); }
		inline typename ArgList::iterator		BeginArg()				{ return m_Args.begin(); }
		inline typename ArgList::const_iterator	EndArg() const			{ return m_Args.end(); }
		inline typename ArgList::iterator		EndArg()				{ return m_Args.end(); }
		inline size_t				GetNumArgs() const		{ return m_Args.size(); }
		inline std::string			GetName() const			{ return m_Name; }
	};
}