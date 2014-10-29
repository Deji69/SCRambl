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
	public:
		typedef std::shared_ptr<Type> Shared;
		typedef std::shared_ptr<const Type> CShared;
		typedef std::unique_ptr<Type> Unique;
		typedef std::unique_ptr<const Type> CUnique;
		enum class ValueType { Null, Value, String, Variable, Label, Enum };

		virtual bool IsBasic() const			{ return false; }
		virtual bool IsExtended() const			{ return false; }
		virtual bool IsVariable() const			{ return false; }

	private:
		class Size
		{
			size_t			m_Size = 0;				// size value
			bool			m_IsVariable = false;	// if true, m_Size is a limit. if false, it's exact

		public:
			Size() = default;
			Size(size_t size, bool variable = false) : m_Size(size), m_IsVariable(variable)
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
			typedef std::shared_ptr<IValue<T>> Shared;
			typedef std::unique_ptr<IValue<T>> Unique;

			IValue(T type) : m_Type(type), m_SizesSorted(true)
			{ }
			inline virtual ~IValue()
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
		class AnonymousValue : public IValue < ValueType >
		{
		public:
			AnonymousValue() : IValue(ValueType::Null)
			{ }
		};


		class Value : public IValue < ValueType >
		{
			bool				m_Float = false;
			bool				m_Packed = false;
			bool				m_Fixed = false;

		public:
			Value() : IValue(ValueType::Value)
			{ }

			inline void SetFloat(bool b)		{ m_Float = b; }
			inline void	SetPacked(bool b)		{ m_Packed = b; }
			inline void	SetFixed(bool b)		{ m_Fixed = b; }
		};

		class String : public IValue < ValueType >
		{
		public:
			String() : IValue(ValueType::String)
			{ }
		};

		class Variable : public IValue < ValueType >
		{
			Type			&	m_ValType;			// INT/FLOAT/etc...
			Type			&	m_VarType;			// VAR/LVAR/etc...

		public:
			Variable(Type & varType, Type & valType) : IValue(ValueType::Variable), m_VarType(varType), m_ValType(valType)
			{ }
		};

		class Label : public IValue < ValueType >
		{
		public:
			Label() : IValue(ValueType::Label)
			{ }
		};

	private:
		unsigned long long						m_ID;
		std::string								m_Name;
		std::vector<IValue<ValueType>::Shared>	m_Values;
		bool									m_Null = true;

	public:
		Type(unsigned long long id, std::string name) : m_ID(id), m_Name(name)
		{ }
		inline virtual ~Type() { }

		inline unsigned long long	GetID() const		{ return m_ID; }
		inline std::string			GetName() const		{ return m_Name; }

		template<typename TType, typename... T>
		IValue<ValueType>::Shared AddValue(T&&... args) {
			auto ptr = std::make_shared<TType>(std::forward<T>(args)...);
			m_Values.emplace_back(ptr);
			return ptr;
		}

		static Shared MakeShared(unsigned long long id, std::string name) {
			return std::make_shared<Type>(id, name);
		}
		static Unique MakeUnique(unsigned long long id, std::string name) {
			return std::make_unique<Type>(id, name);
		}
	};

	enum class VarScope {
		local, global
	};

	// VarType will use VarScope by default to differentiate between variations of variable types
	template<typename = VarScope>
	class VarType;
	
	/*\
	 * SCR::IVarType - Anonymous for SCR::VarType<>
	\*/
	class IVarType
	{
	public:
		template<typename TType>
		inline VarType<TType> & Get()			{ return *static_cast<VarType<TType>>(this); }
	};

	/*\
	 * SCR::VarType<> - SCR variable type
	 * <TType> - Use to override the types of variables - uses local/global scope by default
	\*/
	template<typename TType>
	class VarType : Type
	{
		bool IsVariable() const override			{ return true; }

		TType					m_VarType;			// usually scope
		Type::CShared			m_Type;

	public:
		typedef std::shared_ptr<VarType> Shared;
		typedef std::shared_ptr<const VarType> CShared;
		
		VarType(Type::CShared basic = nullptr) : Type(*basic)
		{ }
		VarType(unsigned long long id, std::string name, TType type) : Type(id, name), m_Type(type)
		{ }
		inline virtual ~VarType() { }

		inline TType GetType() const				{ return m_VarType; }
		inline Type::CShared GetValueType() const	{ return m_Type; }

		inline Type & Disqualify()					{ return *this; }
		inline const Type & Disqualify() const		{ return *this; }

		static Shared MakeShared(unsigned long long id, std::string name, TType var_type, Type::CShared type = nullptr) {
			return std::make_shared<ExtendedType>(id, name, var_type, type);
		}
	};

	/*\
	 * SCR::BasicType - SCR basic type
	\*/
	class BasicType : public Type
	{
		bool IsBasic() const override				{ return true; }

	public:
		typedef std::shared_ptr<BasicType> Shared;
		typedef std::unique_ptr<BasicType> Unique;
		typedef std::shared_ptr<const BasicType> CShared;
		typedef std::unique_ptr<const BasicType> CUnique;

		BasicType(Type::CShared basic = nullptr) : Type(*basic)
		{ }
		BasicType(unsigned long long id, std::string name) : Type(id, name)
		{ }
		inline virtual ~BasicType() { }

		inline Type & Disqualify()					{ return *this; }
		inline const Type & Disqualify() const		{ return *this; }

		static Shared MakeShared(unsigned long long id, std::string name)		{ return std::make_shared<BasicType>(id, name); }
		static Unique MakeUnique(unsigned long long id, std::string name)		{ return std::make_unique<BasicType>(id, name); }
	};

	/*\
	 * SCR::ExtendedType - SCR extended type
	\*/
	class ExtendedType : public Type
	{
		bool IsExtended() const override			{ return true; }

		Type::CShared		m_BasicType;

	public:
		typedef std::shared_ptr<ExtendedType> Shared;
		typedef std::shared_ptr<const ExtendedType> CShared;

		ExtendedType(Type::CShared basic = nullptr) : Type(*basic), m_BasicType(basic)
		{ }
		ExtendedType(unsigned long long id, std::string name, Type::CShared basic = nullptr) : Type(id, name), m_BasicType(basic)
		{ }
		inline virtual ~ExtendedType() { }

		inline Type & Disqualify()					{ return *this; }
		inline const Type & Disqualify() const		{ return *this; }

		static Shared MakeShared(unsigned long long id, std::string name, Type::CShared basic = nullptr) {
			return std::make_shared<ExtendedType>(id, name, basic);
		}
	};
}