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
#include "Attributes.h"

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
		class DataType {
			enum Type {
				Int32, Int24, Int16, Int8,
				Float32, Float24, Float16, Fixed16,
				Char, String
			};

			Type		m_Type;

		public:
			DataType()
			{ }
			DataType(Type type) : m_Type(type)
			{ }

			inline operator const Type&() const			{ return m_Type; }
			inline operator Type&()						{ return m_Type; }
			inline bool IsInteger()	{
				return m_Type == Int32 || m_Type == Int24 || m_Type == Int16 || m_Type == Int8;
			}
			inline bool IsFloat() {
				return m_Type == Float32 || m_Type == Float24 || m_Type == Float16 || m_Type == Fixed16;
			}

			inline static bool GetByName(std::string str, Type & out) {
				static const std::map<std::string, Type> map = {
					{ "Int32", Int32 }, { "Int24", Int24 }, { "Int16", Int16 }, { "Int8", Int8 }, 
					{ "Float32", Float32 }, { "Float24", Float24 }, { "Float16", Float16 }, { "Fixed16", Fixed16 },
					{ "Char", Char }, { "String", String }
				};
				auto it = map.find(str);
				if (it != map.end()) {
					out = it->second;
					return true;
				}
				return false;
			}
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

		enum class DataSourceID {
			None, Value, Number, Text, Command, Variable, Label,
		};
		
		enum class DataAttributeID {
			None, ID, Value, Size, Offset, Index, Name
		};

		/*template<DataAttributeID TID, typename T>
		class DataAttribute : Attribute<TID, T> {
			

		public:
			DataAttribute()
			{ }

		private:
		};*/

		class DataSourceSet : public AttributeSet < DataSourceID >
		{
		public:
			using Shared = std::shared_ptr < DataSourceSet > ;

			DataSourceSet() : AttributeSet(DataSourceID::None)
			{
				AddAttribute("Value", DataSourceID::Value);
				AddAttribute("Number", DataSourceID::Number);
				AddAttribute("Text", DataSourceID::Text);
				AddAttribute("Command", DataSourceID::Command);
				AddAttribute("Variable", DataSourceID::Variable);
				AddAttribute("Label", DataSourceID::Label);
			}
		};

		template<DataSourceID> class DataAttributeSet;

		template<>
		class DataAttributeSet<DataSourceID::Value> : public AttributeSet<DataAttributeID>
		{
		public:
			using Shared = std::shared_ptr < DataAttributeSet > ;

			DataAttributeSet() : AttributeSet(DataAttributeID::None)
			{
				AddAttribute("Value", DataAttributeID::Value);
				AddAttribute("Size", DataAttributeID::Size);
			}
		};

		template<>
		class DataAttributeSet<DataSourceID::Number> : public DataAttributeSet < DataSourceID::Value >
		{ };
		template<>
		class DataAttributeSet<DataSourceID::Text> : public DataAttributeSet < DataSourceID::Value >
		{ };

		template<>
		class DataAttributeSet<DataSourceID::Command> : public AttributeSet<DataAttributeID>
		{
		public:
			using Shared = std::shared_ptr < DataAttributeSet >;

			DataAttributeSet() : AttributeSet(DataAttributeID::None)
			{
				AddAttribute("ID", DataAttributeID::ID);
			}
		};

		template<>
		class DataAttributeSet<DataSourceID::Label> : public AttributeSet<DataAttributeID>
		{
		public:
			using Shared = std::shared_ptr < DataAttributeSet >;

			DataAttributeSet() : AttributeSet(DataAttributeID::None)
			{
				AddAttribute("Offset", DataAttributeID::Offset);
			}
		};

		template<>
		class DataAttributeSet<DataSourceID::Variable> : public AttributeSet<DataAttributeID>
		{
		public:
			using Shared = std::shared_ptr < DataAttributeSet >;

			DataAttributeSet() : AttributeSet(DataAttributeID::None)
			{
				AddAttribute("Index", DataAttributeID::Index);
			}
		};

		/*\
		 * Translation
		\*/
		template<typename TDataType = DataType, typename TDataSource = DataSourceID, typename TDataAttribute = DataAttributeID>
		class Translation
		{
		public:
			using Shared = std::shared_ptr < Translation >;
			
			class Data
			{
			public:
				class Field
				{
				public:
					using Shared = std::shared_ptr<Field>;

					class IValue {
					protected:
						inline virtual ~IValue() { }
					};

					template<typename T>
					class Value : public IValue {
						T			m_Value;

					public:
						Value(const T & val) : m_Value(val)
						{ }

						inline operator T&()				{ return m_Value; }
						inline operator const T&() const	{ return m_Value; }
					};

					Field(TDataType type, TDataSource src, TDataAttribute attr) :
						m_Type(type), m_Source(src), m_Attribute(attr), m_Size(0), m_SizeLimit(false)
					{ }
					Field(TDataType type, TDataSource src, TDataAttribute attr, size_t size) :
						m_Type(type), m_Source(src), m_Attribute(attr), m_Size(size), m_SizeLimit(true)
					{ }

					template<typename T>
					void SetValue(const T & val) {
						m_Value = std::static_pointer_cast<IValue>(std::make_shared<Value<T>>(val));
					}

					template<typename T>
					std::shared_ptr<T>		GetValue() const							{ return std::static_pointer_cast<T>(m_Value); }

					inline TDataType GetDataType() const			{ return m_Type; }
					inline TDataSource GetDataSource() const		{ return m_Source; }
					inline TDataAttribute GetDataAttribute() const	{ return m_Attribute; }
					inline bool HasSizeLimit() const				{ return m_SizeLimit; }
					inline size_t GetSizeLimit() const				{ return m_Size; }

				private:
					TDataType					m_Type;
					TDataSource					m_Source;
					TDataAttribute				m_Attribute;
					bool						m_SizeLimit;
					size_t						m_Size;
					std::shared_ptr<IValue>		m_Value;
				};

				using FieldShared = typename Field::Shared;

				FieldShared AddField(TDataType type, TDataSource src, TDataAttribute attr) {
					auto ptr = std::make_shared<Field>(type, src, attr);
					m_Fields.emplace_back(ptr);
					return ptr;
				}
				FieldShared AddField(TDataType type, TDataSource src, TDataAttribute attr, size_t size) {
					auto ptr = std::make_shared<Field>(type, src, attr, size);
					m_Fields.emplace_back(ptr);
					return ptr;
				}

			private:
				std::vector<std::shared_ptr<Field>>		m_Fields;
			};

			Translation(Type::Shared type, ValueSet valuetype, size_t size) : m_Type(type), m_ValueType(valuetype), m_Size(size)
			{ }

			Data & AddData() {
				m_Data.emplace_back();
				return m_Data.back();
			}

		private:
			Type::Shared			m_Type;
			ValueSet				m_ValueType;
			size_t					m_Size;
			std::vector<Data>		m_Data;
		};

		/*\
		 * AttributeSet of a Value
		\*/
		enum class ValueAttributeID {
			None, Value, Size
		};
		template<typename TAttrID = ValueAttributeID>
		class ValueAttributes : public AttributeSet<TAttrID>
		{

		public:
			ValueAttributes() : AttributeSet(TAttrID::None)
			{
				AddAttribute("Value", TAttrID::Value);
				AddAttribute("Size", TAttrID::Size);
			}
		};

		/*\
		 * Value of Type for translation
		\*/
		class Value : public Type::Value, public ValueAttributes<>
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
			
			inline size_t GetSize() const							{ return m_Size; }
			inline Type::Shared GetType() const						{ return m_Type; }

			inline Translation<>::Shared GetTranslation() const		{ return m_Translation; }
			inline void SetTranslation(Translation<>::Shared v)		{ m_Translation = v; }

			template<typename T>
			inline T& Extend()										{ return *static_cast<T*>(this); }
			template<typename T>
			inline const T& Extend() const							{ return *static_cast<T*>(this); }

		private:
			Type::Shared			m_Type;
			size_t					m_Size;
			Translation<>::Shared	m_Translation = nullptr;
		};

		class ValueToken
		{
		public:
			using Shared = std::shared_ptr < ValueToken > ;

			ValueToken(const Value & value_type) : m_ValueType(value_type)
			{ }
			inline virtual ~ValueToken()
			{ }

		private:
			const Value &		m_ValueType;
		};

		/*\
		 * Number Value
		\*/
		enum class NumberValueType {
			Integer, Float
		};

		enum class NumberValueAttributeID {
			None, Value, Size
		};

		class NumberValueAttributes : public ValueAttributes<NumberValueAttributeID>
		{
		public:
			NumberValueAttributes()
			{ }
		};

		/*class NumberValueToken : ValueToken
		{
		public:
			using Shared = std::shared_ptr < NumberValueToken >;

			template<typename T, typename... TArgs>
			NumberValueToken(const T& info, TArgs&&... args)
			{
				m_NumberType = info.GetValue<Number>();
			}

		private:
			Numbers::Type			m_NumberType;
		};*/

		class NumberValue : public Value
		{
		public:
			using Shared = std::shared_ptr < NumberValue > ;

			NumberValue(Type::Shared type, NumberValueType numtype, size_t size) : Value(type, ValueSet::Number, size), m_Type(numtype)
			{ }

			inline NumberValueType	GetNumberType() const			{ return m_Type; }

			template<typename T = NumberValueToken, typename... TArgs>
			inline std::shared_ptr<T> CreateToken(TArgs&&... args) {
				return std::make_shared<T>(*this, std::forward<TArgs>(args)...);
			}

		private:
			NumberValueType			m_Type;
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
#endif

		class Types {
			Engine					&	m_Engine;
			Configuration::Shared		m_Config;
			Type::Storage				m_Types;
			std::multimap<ValueSet, Value::Shared>	m_Values;
			std::vector<Translation<>::Shared>		m_Translations;
			
		public:
			Types(Engine &);

			DataSourceID GetDataSource(std::string name) {
				static const DataSourceSet s_set;
				return s_set.GetAttribute(name);
			}
			DataAttributeID GetDataAttribute(DataSourceID src, std::string name) {
				switch (src) {
				case DataSourceID::Command:
				{
					static const DataAttributeSet<DataSourceID::Command> s_set;
					return s_set.GetAttribute(name);
				}
				case DataSourceID::Label:
				{
					static const DataAttributeSet<DataSourceID::Label> s_set;
					return s_set.GetAttribute(name);
				}
				case DataSourceID::Number:
				{
					static const DataAttributeSet<DataSourceID::Text> s_set;
					return s_set.GetAttribute(name);
				}
				case DataSourceID::Text:
				{
					static const DataAttributeSet<DataSourceID::Text> s_set;
					return s_set.GetAttribute(name);
				}
				case DataSourceID::Value:
				{
					static const DataAttributeSet<DataSourceID::Value> s_set;
					return s_set.GetAttribute(name);
				}
				case DataSourceID::Variable:
				{
					static const DataAttributeSet<DataSourceID::Variable> s_set;
					return s_set.GetAttribute(name);
				}
				}
				return DataAttributeID::None;
			}

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

			inline Translation<>::Shared AddTranslation(Type::Shared type, ValueSet valuetype, size_t size) {
				m_Translations.emplace_back(std::make_shared<Translation<>>(type, valuetype, size));
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
	}
}