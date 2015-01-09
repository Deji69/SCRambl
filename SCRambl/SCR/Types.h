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
	//class BasicType;
	//class ExtendedType;

	namespace Default {
		enum class Types {
			Standard
		};
		enum class Values {
			Null, Number, Variable, Text, Label
		};

		template<typename TClass, TClass TVal> class Value;

		template<> class Value < Values, Values::Null > {
		public:
			inline Values GetType() const			{ return Values::Null; }
		};

		template<> class Value < Values, Values::Number > {
		public:
			inline Values GetType() const			{ return Values::Number; }
		};

		template<> class Value < Values, Values::Variable > {
		public:
			inline Values GetType() const			{ return Values::Variable; }
		};

		template<> class Value < Values, Values::Text > {
		public:
			inline Values GetType() const			{ return Values::Text; }
		};

		template<> class Value < Values, Values::Label > {
		public:
			inline Values GetType() const			{ return Values::Label; }
		};

		typedef Value<Values, Values::Null> NullValue;
		typedef Value<Values, Values::Number> NumberValue;
		typedef Value<Values, Values::Variable> VariableValue;
		typedef Value<Values, Values::Text> TextValue;
		typedef Value<Values, Values::Label> LabelValue;
	};

	template<typename TType = Default::Types, typename TValType = Default::Values, typename TExtension = void>
	class Type {
	public:
		typedef TExtension Extension;
		typedef std::shared_ptr<Type> Shared;
		typedef std::shared_ptr<const Type> CShared;
		typedef std::unique_ptr<Type> Unique;
		typedef std::unique_ptr<const Type> CUnique;

		template<typename TMainType>
		class MatchSpec {
			TMainType				m_Type;

		public:
			MatchSpec(TMainType type) : m_Type(type)
			{ }

			bool DoesMatch(TMainType type)
			{

			}
		};

		class Value {
		public:
			typedef std::shared_ptr<Value> Shared;

		private:
			TValType			m_ValType;

			inline TValType GetType() const					{ return m_ValType; }

		protected:
			virtual bool TestCompatibility(const Value & lhs) const {
				return true;
			}

		public:
			Value(TValType valtype) : m_ValType(valtype)
			{ }
			inline virtual ~Value() { }

			inline TValType GetValueType() const			{ return m_ValType; }

			template<typename TValueExtension>
			bool IsCompatible(const Value & lhs) const
			{
				return lhs.GetType() == GetType() && TestCompatibility(static_cast<TValueExtension>(lhs));
			}
			template<>
			bool IsCompatible<Value>(const Value & lhs) const
			{
				return lhs.GetType() == GetType() && TestCompatibility(lhs);
			}

			template<typename TExtension>
			inline TExtension		&	Extend()						{ return *static_cast<TExtension*>(this); }
			inline const TExtension	&	Extend() const					{ return *static_cast<TExtension*>(this); }

			static inline typename Type::Shared MakeShared(TValType type)				{ return std::make_shared<Value>(type); }
		};

		typedef typename Value::Shared SharedValue;

		class Storage {
		public:
			typedef std::unordered_map<std::string, std::shared_ptr < Type >> Map;
			typedef std::vector<std::shared_ptr < Type >> Vector;

			Storage() { }

			typename Type::Shared Add(std::string name, TType type) {
				auto ptr = Type::MakeShared(name, type);
				m_Map.emplace(name, ptr);
				m_Vector.push_back(ptr);
				return ptr;
			}
			typename Type::Shared Get(std::string name) {
				auto it = m_Map.find(name);
				return it != m_Map.end() ? it->second : nullptr;
			}

		private:
			Map			m_Map;
			Vector		m_Vector;
		};

		Type(std::string name, TType type) : m_Name(name), m_Type(type)
		{ }
		inline virtual ~Type() { }

		inline Extension		&	Extend()						{ return *static_cast<Extension*>(this); }
		inline const Extension	&	Extend() const					{ return *static_cast<Extension*>(this); }
		inline TType				GetType() const					{ return m_Type; }

		template<typename TValue = Value, typename ...TArgs>
		inline std::shared_ptr<TValue> AddValue(TArgs&&... args) {
			auto shared = std::make_shared<TValue>(std::forward<TArgs>(args)...);
			m_Values.emplace_back(shared);
			return shared;
		}

		/*\
		 * Types::Types::AllValues - Calls the requested function for each Value this Type contains
		\*/
		template<typename TValue = Value, typename TFunc>
		void AllValues(TFunc func) {
			for (auto i : m_Values) {
				func(std::static_pointer_cast<TValue>(i));
			}
		}

		static inline typename Type::Shared MakeShared(std::string name, TType type)			{ return std::make_shared<Type>(name, type); }
		//static inline Type::CShared MakeShared(TType type, TValType valtype) const			{ return std::make_shared<Type>(type, valtype); }

	private:
		std::string						m_Name;
		TType							m_Type;
		std::vector<SharedValue>		m_Values;
	};

	namespace Default
	{
		typedef Type<Types, Values> BasicType;
	};

	// VarType will use VarScope by default to differentiate between variations of variable types
	/*class VarScope;
	template<typename = VarScope>
	class VarType;*/

	/*\
	 * SCR::Type - SCR type
	\*/
	/*class Type
	{
	public:
		typedef std::shared_ptr<Type> Shared;
		typedef std::shared_ptr<const Type> CShared;
		typedef std::unique_ptr<Type> Unique;
		typedef std::unique_ptr<const Type> CUnique;
		
		enum class ValueType { Null, Number, String, Variable, Label, Enum };

		virtual bool IsBasic() const				{ return false; }
		virtual bool IsExtended() const				{ return false; }
		virtual bool IsVariable() const				{ return false; }
		inline BasicType & MakeBasic()				{ return static_cast<BasicType&>(*this); }
		inline BasicType & MakeBasic() const		{ return MakeBasic(); }
		inline ExtendedType & MakeExtended()		{ return static_cast<ExtendedType&>(*this); }
		inline ExtendedType & MakeExtended() const	{ return MakeExtended(); }

		// TODO: Add "IsCompatible" functions
		// TODO: Do proper conversion functions (e.g. converting integers to labels, may entail various checks for validity, etc.)

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

		class Number : public IValue < ValueType >
		{
			bool				m_Float = false;
			bool				m_Packed = false;
			bool				m_Fixed = false;

		public:
			Number() : IValue(ValueType::Number)
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
		Type(Type::Shared basic = nullptr)
		{
			if (basic && !basic->m_Null) {
				for (auto& o : basic->m_Values) {
					m_Values.push_back(std::make_shared<IValue<ValueType>>(o));
				}
				m_Null = false;
			}
		}
		Type(unsigned long long id, std::string name, Type::CShared basic = nullptr) : m_ID(id), m_Name(name)
		{
			if (basic && !basic->m_Null) {
				for (auto& o : basic->m_Values) {
					m_Values.push_back(std::make_shared<IValue<ValueType>>(o));
				}
				m_Null = false;
			}
		}
		inline virtual ~Type() { }

		inline unsigned long long	GetID() const		{ return m_ID; }
		inline std::string			GetName() const		{ return m_Name; }

		template<typename TType, typename... T>
		IValue<ValueType>::Shared AddValue(T&&... args) {
			auto ptr = std::make_shared<TType>(std::forward<T>(args)...);
			m_Values.emplace_back(ptr);
			m_Null = false;
			return ptr;
		}

	protected:
		static Shared MakeShared(unsigned long long id, std::string name) {
			return std::make_shared<Type>(id, name);
		}
		static Unique MakeUnique(unsigned long long id, std::string name) {
			return std::make_unique<Type>(id, name);
		}
	};

	class VarScope {
	public:
		enum Type {
			invalid, local, global
		};

		static inline std::string GetTypeName(VarScope type) {
			return type == invalid ? "" : (type == global ? "global" : "local");
		}

		inline std::string GetName() const		{ return GetTypeName(m_Type); }
		inline bool IsValid() const				{ return m_Type != invalid; }
		
		inline operator Type() const			{ return m_Type; }
		inline operator bool() const			{ return IsValid(); }

		VarScope() = default;
		VarScope(Type scope) : m_Type(scope)
		{ }
		VarScope(std::string scope) : m_Type(scope != "global" ? (scope == "local" ? local : invalid) : global)
		{ }

	private:
		Type		m_Type = invalid;
	};

	static std::string GetVarTypeName(VarScope type);*/
	
	/*\
	 * SCR::IVarType - Anonymous for SCR::VarType<>
	\*/
	/*class IVarType
	{
	public:
		template<typename TType>
		inline VarType<TType> & Get()			{ return *static_cast<VarType<TType>>(this); }
	};*/

	/*\
	 * SCR::VarType<> - SCR variable type
	 * <TType> - Use to override the types of variables - uses local/global scope by default
	\*/
	/*template<typename TType>
	class VarType : public Type
	{
		bool IsVariable() const override			{ return true; }

		TType					m_VarType;				// class of variable - usually scope
		Type::Shared			m_Type;					// applicable for arrays
		bool					m_IsArray = false;
		//short					m_Width = 0;			// capacity in bits

	protected:
		inline void SetWidth(short w)				{ m_Width = w; }
		inline void SetIsArray(bool b)				{ m_IsArray = b; }

	public:
		typedef std::shared_ptr<VarType> Shared;
		typedef std::shared_ptr<const VarType> CShared;
		
		VarType(Type::Shared basic = nullptr) : Type(basic)
		{ }
		VarType(unsigned long long id, std::string name, TType type, Type::CShared basic = nullptr) : Type(id, name), m_VarType(type), m_Type(basic)
		{ }
		inline virtual ~VarType() { }

		inline short GetWidth() const				{ return m_Width; }
		inline bool IsArray() const					{ return m_IsArray; }

		inline TType GetType() const				{ return m_VarType; }
		inline Type::CShared GetValueType() const	{ return m_Type; }
		inline Type::Shared GetValueType()			{ return m_Type; }

		inline Type & Disqualify()					{ return *this; }
		inline const Type & Disqualify() const		{ return *this; }

		static Shared MakeShared(unsigned long long id, std::string name, TType var_type, Type::CShared type = nullptr) {
			return std::make_shared<VarType>(id, name, var_type, type);
		}
	};*/

	/*\
	 * SCR::BasicType - SCR basic type
	\*/
	/*class BasicType : public Type
	{
		bool IsBasic() const override				{ return true; }

	public:
		typedef std::shared_ptr<BasicType> Shared;
		typedef std::unique_ptr<BasicType> Unique;
		typedef std::shared_ptr<const BasicType> CShared;
		typedef std::unique_ptr<const BasicType> CUnique;

		BasicType(Type::Shared basic = nullptr) : Type(*basic)
		{ }
		BasicType(unsigned long long id, std::string name) : Type(id, name)
		{ }
		inline virtual ~BasicType() { }

		inline Type & Disqualify()					{ return *this; }
		inline const Type & Disqualify() const		{ return *this; }

		static Shared MakeShared(unsigned long long id, std::string name)		{ return std::make_shared<BasicType>(id, name); }
		static Unique MakeUnique(unsigned long long id, std::string name)		{ return std::make_unique<BasicType>(id, name); }
	};*/

	/*\
	 * SCR::ExtendedType - SCR extended type
	\*/
	/*class ExtendedType : public Type
	{
		bool IsExtended() const override			{ return true; }

	public:
		typedef std::shared_ptr<ExtendedType> Shared;
		typedef std::shared_ptr<const ExtendedType> CShared;

	private:
		ExtendedType::Shared	m_This;
		Type::Shared			m_BasicType;

	public:
		ExtendedType(Type::Shared basic = nullptr) : Type(basic), m_BasicType(basic), m_This(this)
		{ }
		ExtendedType(unsigned long long id, std::string name, Type::CShared basic = nullptr) : Type(id, name, basic), m_BasicType(basic), m_This(this)
		{ }
		inline virtual ~ExtendedType() { }

		inline ExtendedType::Shared ShareExtended()			{ return m_This; }
		inline ExtendedType::CShared ShareExtended() const	{ return m_This; }
		inline Type::CShared GetBasic() const				{ return m_BasicType; }

		inline Type & Disqualify()							{ return *this; }
		inline const Type & Disqualify() const				{ return *this; }

		static Shared MakeShared(unsigned long long id, std::string name, Type::CShared basic = nullptr) {
			return std::make_shared<ExtendedType>(id, name, basic);
		}
	};*/
}