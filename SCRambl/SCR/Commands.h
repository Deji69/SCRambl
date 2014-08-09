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
			bool					m_IsReturn = true;


		public:
			Arg(const Type & type) : m_Type(type)
			{ }

			inline bool			IsReturn() const		{ return m_IsReturn; }
		};

	private:
		//uint64_t				m_Index;
		unsigned long long		m_Index;				// unique index/hash
		std::string				m_Name;					// command name/id
		std::vector<Arg>		m_Args;

	public:
		Command(std::string name, unsigned long long index) : m_Name(name), m_Index(index)
		{
		}

		void AddArg() {
		}

		inline std::string		GetName() const			{ return m_Name; }
	};
}