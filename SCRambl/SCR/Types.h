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

namespace SCR
{
	/*\
	 * SCR::Type - SCR type
	\*/
	class Type
	{
		unsigned long long			m_ID;
		std::string					m_Name;

	public:
		Type(unsigned long long id, std::string name) : m_ID(id), m_Name(name)
		{ }

		inline unsigned long long	GetID() const		{ return m_ID; }
		inline const std::string &	GetName() const		{ return m_Name; }
	};
}