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
	 * Command - SCR command
	\*/
	class Command
	{
	public:
		using Shared = std::shared_ptr < SCR::Command > ;

		class Arg
		{
			const Type			&	m_Type;
			int						m_Index;				// nth arg
			//std::string			m_Description;
			bool					m_IsReturn = false;

		public:
			using List = std::vector < Arg >;
			using Iterator = List::iterator;
			using CIterator = List::const_iterator;
			using RIterator = List::reverse_iterator;
			using CRIterator = List::const_reverse_iterator;

			Arg(const Type & type) : m_Type(type)
			{ }

			inline bool			IsReturn() const		{ return m_IsReturn; }
			inline int			GetIndex() const		{ return m_Index; }
			inline const Type & GetType() const			{ return m_Type; }
		};

		typedef std::vector<Arg> ArgList;

	private:
		//uint64_t				m_Index;
		unsigned long long		m_Index;				// unique index/hash
		std::string				m_Name;					// command name/id
		ArgList					m_Args;

	public:
		Command(std::string name, unsigned long long index) : m_Name(name), m_Index(index)
		{
		}

		void AddArg(const Type & type) {
			m_Args.emplace_back(type);
		}

		inline Arg::CIterator	BeginArg() const		{ return m_Args.begin(); }
		inline Arg::Iterator	BeginArg()				{ return m_Args.begin(); }
		inline Arg::CIterator	EndArg() const			{ return m_Args.end(); }
		inline Arg::Iterator	EndArg()				{ return m_Args.end(); }
		inline std::string		GetName() const			{ return m_Name; }
	};
}