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
		inline virtual ~Type() { }

		inline unsigned long long	GetID() const		{ return m_ID; }
		inline std::string			GetName() const		{ return m_Name; }
	};

	/*\
	 * SCR::BasicType - SCR basic type
	\*/
	class BasicType : public Type
	{
	public:
		enum class ValueType { Value, String, Variable, Label, Enum };

	private:
		class Size
		{
			size_t			m_Size;					// size value
			bool			m_IsVariable;			// if true, m_Size is a limit. if false, it's exact

		public:
			Size(size_t size, bool variable = false): m_Size(size), m_IsVariable(variable)
			{ }

			inline size_t GetSize() const		{ return m_Size; }
			inline size_t IsVariable() const	{ return m_IsVariable; }
		};

		template<typename T>
		class IValue
		{
			std::vector<Size>	m_Sizes;
			T					m_Type;

			bool				m_SizesSorted = true;

		public:
			IValue(T type) : m_Type(type), m_SizesSorted(true)
			{ }
			virtual ~IValue()
			{ }

			inline T GetType() const			{ return m_Type; }

			template<typename... T>
			inline void	AddSize(T&&... args) {
				m_Sizes.emplace_back(std::forward<T>(args)...);
				m_SizesSorted = false;
			}
			inline void	SortSizes() {
				if (!m_SizesSorted) {
					std::sort(std::begin(m_Sizes), std::end(m_Sizes), [](const Size & a, const Size & b){ return a.GetSize() < b.GetSize(); });
					m_SizesSorted = true;
				}
			}
		};

	public:
		class Value : public IValue < ValueType >
		{
			bool				m_Float = false;
			bool				m_Packed = false;
			bool				m_Fixed = false;

		public:
			Value(): IValue(ValueType::Value)
			{ }

			inline void SetFloat(bool b)		{ m_Float = b; }
			inline void	SetPacked(bool b)		{ m_Packed = b; }
			inline void	SetPacked(bool b)		{ m_Fixed = b; }
		};

		class String : public IValue < ValueType >
		{
		public:
		};

		class Variable : public IValue < ValueType >
		{
			Type			&	m_ValType;			// INT/FLOAT/etc...
			Type			&	m_VarType;			// VAR/LVAR/etc...

		public:
			Variable(Type & varType, Type & valType): IValue(ValueType::Variable), m_VarType(varType), m_ValType(valType)
			{ }
		};

		class Label : public IValue < ValueType >
		{
		public:
			Label(): IValue(ValueType::Label)
			{ }
		};

		BasicType(unsigned long long id, std::string name) : Type(id, name)
		{ }
		inline virtual ~BasicType() { }
	};
}