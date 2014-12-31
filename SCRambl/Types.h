/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include "utils.h"
#include "Configuration.h"
#include "SCR.h"

namespace SCRambl
{
	class Engine;

	namespace Types
	{
		typedef unsigned long long TypeID;

		// Type set
		enum class TypeSet {
			Basic, Extended, Variable
		};

		// Value set
		enum class ValueSet {
			Null, Number, Variable, Text, Label,
			INVALID
		};
		static ValueSet		GetValueTypeByName(std::string name);

		// Internal data types
		enum class DataType {
			Int32, Int24, Int16, Int8,
			Float32, Float24, Float16, Fixed16,
			Char, Text
		};

		/*\
		 * Types::Type - SCR Type extension
		\*/
		class Type : public SCR::Type<TypeSet, ValueSet, Type> {
		public:
			using Value = Type::Value;

			//typedef std::shared_ptr<TypeExt> Shared;

			/*inline bool IsNull() const					{ return GetValueType() == ValueSet::Null; }
			inline bool IsNumber() const				{ return GetValueType() == ValueSet::Number; }
			inline bool IsVariable() const				{ return GetValueType() == ValueSet::Variable; }
			inline bool IsText() const					{ return GetValueType() == ValueSet::Text; }
			inline bool IsLabel() const					{ return GetValueType() == ValueSet::Label; }*/
			inline bool IsBasicType() const				{ return GetType() == TypeSet::Basic; }
			inline bool IsExtendedType() const			{ return GetType() == TypeSet::Extended; }
			inline bool IsVariableType() const			{ return GetType() == TypeSet::Variable; }
		};

		//typedef SCR::Type<TypeSet, Values, Type> SCRType;

		class TypeCompat {
		public:
			enum CompatibilityLevel {
				Incompatible,
				CompatibleWithConversion,
				Compatible,
			};

			inline CompatibilityLevel& GetCompatLevel()						{ return m_Level; }
			inline const CompatibilityLevel& GetCompatLevel() const			{ return m_Level; }

			inline bool IsFullyCompatible() const							{ return GetCompatLevel() == Compatible; }
			inline bool IsBasicallyCompatible() const						{ return GetCompatLevel() != Incompatible; }

			inline operator CompatibilityLevel&()							{ return GetCompatLevel(); }
			inline operator const CompatibilityLevel&() const				{ return GetCompatLevel(); }

			TypeCompat(CompatibilityLevel lvl) : m_Level(lvl)
			{ }

		private:
			CompatibilityLevel			m_Level;
		};

		/*\
		 * Translation
		\*/
		class Translation
		{
		public:
			using Shared = std::shared_ptr < Translation >;

			Translation(Type::Shared type, ValueSet valuetype, size_t size) : m_Type(type), m_ValueType(valuetype), m_Size(size)
			{ }

		private:
			Type::Shared			m_Type;
			ValueSet				m_ValueType;
			size_t					m_Size;
		};

		/*\
		 * Value of Type for translation
		\*/
		class Value : public Type::Value
		{
		public:
			using Shared = std::shared_ptr < Value > ;

			Value(Type::Shared type, ValueSet valtype) : Type::Value(valtype), m_Type(type), m_Size(0)
			{ }
			Value(Type::Shared type, ValueSet valtype, size_t size) : Type::Value(valtype), m_Type(type), m_Size(size)
			{ }
			inline virtual ~Value() { }

			virtual bool CanFitSize(size_t size) {
				return GetSize() >= size;
			}

			inline size_t GetSize() const						{ return m_Size; }
			inline Type::Shared GetType() const					{ return m_Type; }

			inline Translation::Shared GetTranslation() const	{ return m_Translation; }
			inline void SetTranslation(Translation::Shared v)	{ m_Translation = v; }

		private:
			Type::Shared			m_Type;
			size_t					m_Size;
			Translation::Shared		m_Translation = nullptr;
		};

		/*\
		 * Number Value
		\*/
		class NumberValue : public Value
		{
		public:
			enum NumberType {
				Integer, Float
			};

			using Shared = std::shared_ptr < NumberValue > ;

			NumberValue(Type::Shared type, NumberType numtype, size_t size) : Value(type, ValueSet::Number, size), m_Type(numtype)
			{ }

			inline NumberType	GetNumberType() const			{ return m_Type; }

		private:
			NumberType			m_Type;
		};

		/*\
		 * Label Value
		\*/
		class LabelValue : Value
		{
		};

#if 0
		template<typename T = NumberValue>
		class ValueExt {
		public:
			enum class SizeMode {
				Fixed, Variable, Packed
			};

			ValueExt(Values valtype)
			{ }

			inline virtual ~ValueExt() { }

		protected:
			virtual bool TestCompatibility(const ValueExt & v) const {
				return CheckCompatibility(v);
			}
			inline bool CheckCompatibility(const ValueExt & v) const {
				return m_Value == v;
			}

		private:
			inline bool CheckSizeModeCompatibility(const ValueExt & v) const {
				return true || m_SizeMode == v.m_SizeMode;
			}

			T					m_Value;
			SizeMode			m_SizeMode = SizeMode::Fixed;
		};

		class NumberValue : public Type::Value {
		public:
			enum Type {
				Integer, Float
			};

			NumberValue(Type num_type, size_t num_bits) : Value(ValueSet::Number), m_Type(num_type), m_NumBits(num_bits)
			{ }

		private:
			Type			m_Type;
			size_t			m_NumBits;

			inline Type		GetType() const				{ return m_Type; }
			inline size_t	GetNumBits() const			{ return m_NumBits; }

			virtual TypeCompat CheckCompatibility(const NumberValue & rhs) const {
				if (GetType() != rhs.GetType())
					return TypeCompat::CompatibleWithConversion;

				// if there's not enuf bits, this ain't compatible
				if (GetNumBits() < rhs.GetNumBits())
					return TypeCompat::Incompatible;

				return TypeCompat::Compatible;
			}

		public:
			inline bool operator==(const NumberValue & rhs) const {
				return CheckCompatibility(rhs) != TypeCompat::Incompatible;
			}
		};

		template<typename T>
		class NumberValueExt : public ValueExt {
		public:
			NumberValueExt(T type) : ValueExt(ValueSet::Number), m_Type(type)
			{ }
			inline virtual ~NumberValueExt() { }

		protected:
			virtual bool TestCompatibility(const Value & lhs) const final override {

				return true;
			}

		private:
			T				m_Type;
		};

		template<typename T>
		class TextValueExt : ValueExt {
		public:
			TextValueExt(T type) : Value(ValueSet::Text), m_Type(type)
			{ }
			inline virtual ~TextValueExt() { }

		private:
			T				m_Type;
		};
#endif

		class Types {
			Engine					&	m_Engine;
			Configuration::Shared		m_Config;
			Type::Storage				m_Types;
			std::multimap<ValueSet, Value::Shared>	m_Values;
			std::vector<Translation::Shared>		m_Translations;

		public:
			Types(Engine &);

			inline Type::Shared AddType(std::string name, TypeSet typeset) {
				auto shared = m_Types.Add(name, typeset);
				return shared;
			}
			inline Type::Shared GetType(std::string name) {
				return m_Types.Get(name);
			}
			inline void AddValue(ValueSet valtype, Value::Shared value) {
				m_Values.emplace(valtype, value);
			}

			inline Translation::Shared AddTranslation(Type::Shared type, ValueSet valuetype, size_t size) {
				m_Translations.emplace_back(type, valuetype, size);
				return m_Translations.back();
			}

			/*\
			 * Types::Types::AllValues - Calls the requested function for each value of 'valtype'
			 * Returns the number of values found
			\*/
			template<typename TFunc>
			size_t AllValues(ValueSet valtype, TFunc func) const {
				auto rg = m_Values.equal_range(valtype);
				int n = 0;
				for (auto i = rg.first; i != rg.second; ++i) {
					func(i->second);
				}
				return n;
			}

			/*\
			 * Types::Types::GetValues - Pushes out a list of values of 'valtype' which contain values of at least 'size' in bits
			 * Returns the number of values added to the list
			 * Optional bool(Value::Shared) function which should return true if it wants to push the Value
			\*/
			size_t GetValues(ValueSet valtype, size_t size, std::vector<Value::Shared> & out) const {
				int i = 0;
				AllValues(valtype, [&](Value::Shared value){
					if (value->CanFitSize(size)) {
						out.push_back(value);
						++i;
					}
				});
				return i;
			}
			template<typename TFunc>
			size_t GetValues(ValueSet valtype, size_t size, std::vector<Value::Shared> & out, TFunc func) const {
				int i = 0;
				AllValues(valtype, [&](Value::Shared value){
					if (value->CanFitSize(size) && func(value)) {
						out.push_back(value);
						++i;
					}
				});
				return i;
			}
		};


		/*typedef SCR::Default::Value<Values, ValueSet::Null> NullValue;
		typedef SCR::Default::Value<Values, ValueSet::Number> NumberValue;
		typedef SCR::Default::Value<Values, ValueSet::Variable> VariableValue;
		typedef SCR::Default::Value<Values, ValueSet::Text> TextValue;
		typedef SCR::Default::Value<Values, ValueSet::Label> LabelValue;*/

		/*\
		 * Types - SCR type manager
		\*/
		/*class Types
		{
		public:
			typedef std::unordered_map < std::string, SCR::Type<>::Shared > Map;

		private:
			Engine					&	m_Engine;
			Configuration::Shared		m_Config;
			Map							m_Map;

		public:
			Types(Engine & eng);

			inline SCR::Type<>::Shared AddType(std::string name, unsigned long long id)
			{
				auto type = SCR::BasicType::MakeShared(id, name);
				m_Map.emplace(name, type);
				return type;
			}
			inline SCR::Type::Shared AddExtendedType(std::string name, unsigned long long id, SCR::Type::CShared type = nullptr)
			{
				auto full_type = SCR::ExtendedType::MakeShared(id, name, type);
				m_Map.emplace(name, full_type);
				return full_type;
			}
			template<typename T>
			inline SCR::Type::Shared AddVariableType(std::string name, unsigned long long id, T var_type, SCR::Type::CShared type = nullptr)
			{
				auto vartype = SCR::VarType<T>::MakeShared(id, name, var_type, type);
				m_Map.emplace(name, vartype);
				return vartype;
			}
			inline SCR::Type::Shared GetType(const std::string & name) const
			{
				if (m_Map.empty()) return nullptr;
				auto it = m_Map.find(name);
				if (m_Map.end() == it) return nullptr;
				return it->second;
			}
		};*/
	}
}