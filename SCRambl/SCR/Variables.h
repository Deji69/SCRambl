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
#include <algorithm>
#include <vector>
#include "Types.h"

namespace SCR
{
	template<typename TType = Default::BasicType>
	class Variable
	{
		std::string					m_Name;
		typename TType::Shared		m_Type;

	public:
		Variable(std::string name, typename TType::Shared type) : m_Name(name), m_Type(type)
		{ }

		inline typename TType::Shared Type() const			{ return m_Type; }
		inline const std::string& Name() const				{ return m_Name; }
	};
}