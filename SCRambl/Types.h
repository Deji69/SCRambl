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
#include "Tokens.h"
#include "TokensB.h"

namespace SCRambl
{
	class Build;

	namespace Types
	{
		typedef unsigned long long TypeID;

		enum class TypeSet {
			Basic, Extended, Variable
		};
		enum class ValueSet {
			Null, Number, Variable, Array, Text, Label, Command,
			INVALID
		};

		static ValueSet GetValueTypeByName(std::string name);

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
			inline bool IsInteger() const	{
				return m_Type == Int32 || m_Type == Int24 || m_Type == Int16 || m_Type == Int8;
			}
			inline bool IsFloat() const {
				return m_Type == Float32 || m_Type == Float24 || m_Type == Float16 || m_Type == Fixed16;
			}
			inline bool IsString() const {
				return m_Type == String;
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

		class Basic;
		class Extended;
		class Variable;

		/* Types::Type */
		class Type {
		public:
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
			private:
				ValueSet m_ValType;

				inline ValueSet GetType() const { return m_ValType; }

			protected:
				virtual bool TestCompatibility(const Value &) const {
					return true;
				}

			public:
				Value(ValueSet valtype) : m_ValType(valtype)
				{ }
				inline virtual ~Value() { }

				inline ValueSet GetValueType() const {
					return m_ValType;
				}

				template<typename TValueExtension>
				bool IsCompatible(const Value & lhs) const {
					return lhs.GetType() == GetType() && TestCompatibility(static_cast<TValueExtension>(lhs));
				}
				template<>
				bool IsCompatible<Value>(const Value & lhs) const {
					return lhs.GetType() == GetType() && TestCompatibility(lhs);
				}
			};

			//using SharedValue = Value::Shared;

			Type(std::string name, TypeSet type) : m_Name(name), m_Type(type)
			{ }
			inline virtual ~Type() { }

			virtual bool IsGlobal() const { return true; }
			inline bool IsScoped() const { return !IsGlobal(); }

			inline std::string GetName() const { return m_Name; }
			inline TypeSet GetType() const { return m_Type; }
			bool IsBasicType() const;
			bool IsExtendedType() const;
			bool IsVariableType() const;
			Basic* ToBasic();
			Extended* ToExtended();
			Variable* ToVariable();
			const Basic* ToBasic() const;
			const Extended* ToExtended() const;
			const Variable* ToVariable() const;

			bool HasValueType(ValueSet type) const;
			template<typename TValue = Value>
			inline std::vector<TValue*> GetValueTypes(ValueSet type) const {
				std::vector<TValue*> vec;
				for (auto& val : m_Values) {
					if (val.GetValueType() == type)
						vec.emplace_back(&val);
				}
				return vec;
			}
			
			template<typename TValue = Value, typename ...TArgs>
			inline TValue* AddValue(TArgs&&... args) {
				m_Values.push_back(std::make_shared<TValue>(std::forward<TArgs>(args)...));
				return static_cast<TValue*>(m_Values.back().get());
			}

			/*\ Types::Types::AllValues - Calls the requested function for each Value this Type contains \*/
			template<typename TValue = Value, typename TFunc>
			void AllValues(TFunc func) {
				for (auto v : m_Values) {
					func(static_cast<TValue*>(v.get()));
				}
			}

		private:
			std::string m_Name;
			TypeSet m_Type;
			std::vector<std::shared_ptr<Value>> m_Values;
		};

		class Basic : public Type
		{
		public:
			using Shared = std::shared_ptr<Basic>;

			Basic(std::string name) : Type(name, TypeSet::Basic)
			{ }
		
		protected:
			Basic(std::string name, bool) : Type(name, TypeSet::Extended)
			{ }
		};
		class Variable : public Type
		{
			XMLValue m_Scope;
			XMLValue m_IsArray;

			// <MinIndex>
			XMLValue m_MinIndex = 1;
			// <MaxIndex>
			XMLValue m_MaxIndex = LONG_MAX;

		public:
			using Shared = std::shared_ptr<Variable>;

			Variable(std::string name, XMLValue scope, XMLValue isarray) : Type(name, TypeSet::Variable),
				m_Scope(scope), m_IsArray(isarray)
			{ }

			XMLValue Scope() const { return m_Scope; }
			bool IsGlobal() const { return lengthcompare(m_Scope.AsString("local"), "local") == 0; }
			XMLValue IsArray() const { return m_IsArray; }
			XMLValue MinIndex() const { return m_MinIndex; }
			XMLValue MaxIndex() const { return m_MinIndex; }
		};
		class Extended : public Type
		{
			Type* m_BasicType;

		public:
			Extended(std::string name, Type* basic = nullptr) : Type(name, TypeSet::Extended),
				m_BasicType(basic)
			{ }
		};

		class Storage {
		public:
			using Map = std::unordered_map<std::string, Type*>;
			using Vector = std::vector<Type*>;

			Storage() { }

			template<typename T>
			void Add(std::string name, T* obj) {
				m_Vector.emplace_back(obj);
				m_Map.emplace(name, obj);
			}
			inline Basic* AddBasic(std::string name) {
				m_Basics.emplace_back(name);
				auto ptr = &m_Basics.back();
				Add<Basic>(name, ptr);
				return ptr;
			}
			inline Extended* AddExtended(std::string name, Basic* basic = nullptr) {
				m_Extendeds.emplace_back(name, basic);
				auto ptr = &m_Extendeds.back();
				Add<Extended>(name, ptr);
				return ptr;
			}
			inline Variable* AddVariable(std::string name, XMLValue scope, bool is_array = false) {
				m_Variables.emplace_back(name, scope, is_array);
				auto ptr = &m_Variables.back();
				Add<Variable>(name, ptr);
				return ptr;
			}
			Type* Get(std::string name) {
				auto it = m_Map.find(name);
				return it != m_Map.end() ? it->second : nullptr;
			}

		private:
			Map m_Map;
			Vector m_Vector;
			std::vector<Basic> m_Basics;
			std::vector<Extended> m_Extendeds;
			std::vector<Variable> m_Variables;
		};

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
			using Shared = std::shared_ptr<DataAttributeSet>;

			DataAttributeSet() : AttributeSet(DataAttributeID::None)
			{
				AddAttribute("Value", DataAttributeID::Value);
				AddAttribute("Size", DataAttributeID::Size);
			}
		};

		template<>
		class DataAttributeSet<DataSourceID::Number> : public DataAttributeSet<DataSourceID::Value>
		{ };
		template<>
		class DataAttributeSet<DataSourceID::Text> : public DataAttributeSet<DataSourceID::Value>
		{ };

		template<>
		class DataAttributeSet<DataSourceID::Command> : public AttributeSet<DataAttributeID>
		{
		public:
			using Shared = std::shared_ptr<DataAttributeSet>;

			DataAttributeSet() : AttributeSet(DataAttributeID::None)
			{
				AddAttribute("ID", DataAttributeID::ID);
			}
		};

		template<>
		class DataAttributeSet<DataSourceID::Label> : public AttributeSet<DataAttributeID>
		{
		public:
			using Shared = std::shared_ptr<DataAttributeSet>;

			DataAttributeSet() : AttributeSet(DataAttributeID::None)
			{
				AddAttribute("Offset", DataAttributeID::Offset);
			}
		};

		template<>
		class DataAttributeSet<DataSourceID::Variable> : public AttributeSet<DataAttributeID>
		{
		public:
			using Shared = std::shared_ptr<DataAttributeSet>;

			DataAttributeSet() : AttributeSet(DataAttributeID::None)
			{
				AddAttribute("Index", DataAttributeID::Index);
			}
		};

		/*\ Translation \*/
		//template<typename TDataType = DataType, typename TDataSource = DataSourceID, typename TDataAttribute = DataAttributeID>
		class Translation
		{
		public:
			class Data
			{
			public:
				class Field
				{
				public:
					class IValue {
					public:
						inline virtual ~IValue() { }
					};

					template<typename T>
					class Value : public IValue {
						T m_Value;

					public:
						Value(const T& val) : m_Value(val)
						{ }

						inline operator T&() { return m_Value; }
						inline operator const T&() const { return m_Value; }
					};

					Field(DataType type, DataSourceID src, DataAttributeID attr) :
						m_Type(type), m_Source(src), m_Attribute(attr), m_Size(0), m_SizeLimit(false)
					{ }
					Field(DataType type, DataSourceID src, DataAttributeID attr, size_t size) :
						m_Type(type), m_Source(src), m_Attribute(attr), m_Size(size), m_SizeLimit(true)
					{ }
					
					template<typename T>
					void SetValue(const T& val) {
						m_Value = std::make_shared<Value<T>>(val);
					}

					template<typename T>
					T* GetValue() const { return static_cast<T*>(m_Value.get()); }

					inline DataType GetDataType() const { return m_Type; }
					inline DataSourceID GetDataSource() const { return m_Source; }
					inline DataAttributeID GetDataAttribute() const { return m_Attribute; }
					inline bool HasSizeLimit() const { return m_SizeLimit; }
					inline size_t GetSizeLimit() const { return m_Size; }

				private:
					DataType m_Type;
					DataSourceID m_Source;
					DataAttributeID m_Attribute;
					bool m_SizeLimit;
					size_t m_Size;
					std::shared_ptr<IValue> m_Value;
				};

				Data() = default;

				Field* AddField(DataType type, DataSourceID src, DataAttributeID attr) {
					m_Fields.emplace_back(type, src, attr);
					return &m_Fields.back();
				}
				Field* AddField(DataType type, DataSourceID src, DataAttributeID attr, size_t size) {
					m_Fields.emplace_back(type, src, attr, size);
					return &m_Fields.back();
				}

			private:
				std::vector<Field> m_Fields;
			};

			Translation(Type* type, ValueSet valuetype, size_t size) : m_Type(type), m_ValueType(valuetype), m_Size(size)
			{ }

			Data& AddData() {
				m_Data.emplace_back();
				return m_Data.back();
			}

		private:
			Type* m_Type;
			ValueSet m_ValueType;
			size_t m_Size;
			std::vector<Data> m_Data;
		};

		/*\ AttributeSet of a Value \*/
		enum class ValueAttributeID {
			None, Value, Size
		};
		class ValueAttributes : public AttributeSet<ValueAttributeID>
		{

		public:
			ValueAttributes() : AttributeSet(ValueAttributeID::None) {
				AddAttribute("Value", ValueAttributeID::Value);
				AddAttribute("Size", ValueAttributeID::Size);
			}
		};

		/*\ Value of Type for translation \*/
		class Value : public Type::Value, public ValueAttributes
		{
		public:
			Value(Type* type, ValueSet valtype) : Type::Value(valtype), m_Type(type), m_Size(0)
			{ }
			Value(Type* type, ValueSet valtype, size_t size) : Type::Value(valtype), m_Type(type), m_Size(size)
			{ }
			inline virtual ~Value() { }

			virtual bool CanFitSize(size_t size) {
				return GetSize() >= size;
			}
			
			inline size_t GetSize() const { return m_Size; }
			inline Type* GetType() const { return m_Type; }

			inline Translation* GetTranslation() const { return m_Translation; }
			inline void SetTranslation(Translation* v) { m_Translation = v; }

			template<typename T>
			inline T& Extend() { return *static_cast<T*>(this); }
			template<typename T>
			inline const T& Extend() const { return *static_cast<T*>(this); }

		private:
			Type* m_Type;
			size_t m_Size;
			Translation* m_Translation = nullptr;
		};

		/*\ Number Value \*/
		enum class NumberValueType {
			Integer, Float
		};

		enum class NumberValueAttributeID {
			None, Value, Size
		};

		class NumberValueAttributes : public ValueAttributes
		{
		public:
			NumberValueAttributes()
			{ }
		};

		class NumberValue : public Value
		{
		public:
			NumberValue(Type* type, NumberValueType numtype, size_t size) : Value(type, ValueSet::Number, size), m_Type(numtype)
			{ }

			inline NumberValueType	GetNumberType() const			{ return m_Type; }

			template<typename T, typename... TArgs>
			inline std::shared_ptr<T> CreateToken(TArgs&&... args) {
				return std::make_shared<T>(Tokens::Type::Number, *this, std::forward<TArgs>(args)...);
			}

		private:
			NumberValueType	m_Type;
		};
		class TextValue : public Value {
		public:
			TextValue(Type* type, size_t size, XMLValue mode = "", XMLValue terminate = "") : Value(type, ValueSet::Text, size),
				m_Mode(mode), m_Terminate(terminate)
			{ }

		private:
			XMLValue m_Mode;
			XMLValue m_Terminate;
		};
		class LabelValue : public Value
		{
		public:
			LabelValue(Type* type, size_t size) : Value(type, ValueSet::Label, size)
			{ }
		};
		class CommandValue : public Value
		{
		public:
			CommandValue(Type* type, size_t size) : Value(type, ValueSet::Command, size)
			{ }
		};
		class VariableValue : public Value {
		public:
			VariableValue(Type* type, size_t size) : Value(type, ValueSet::Variable, size)
			{ }
		};
		class ArrayValue : public Value {
		public:
			ArrayValue(Type* type, size_t size) : Value(type, ValueSet::Array, size)
			{ }
		};

		class Types {
			XMLConfiguration* m_Config;
			Storage m_Types;
			std::multimap<ValueSet, Value*> m_Values;
			std::vector<Translation> m_Translations;
			
		public:
			Types();
			
			void Init(Build&);

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

			inline Type* AddType(std::string name) {
				auto shared = m_Types.AddBasic(name);
				return shared;
			}
			inline Extended* AddExtendedType(std::string name, Basic* basic = nullptr) {
				auto type = m_Types.AddExtended(name, basic);
				return type;
			}
			inline Variable* AddVariableType(std::string name, XMLValue scope, bool is_array = false) {
				auto type = m_Types.AddVariable(name, scope, is_array);
				return type;
			}
			inline Type* GetType(std::string name) {
				return m_Types.Get(name);
			}
			inline void AddValue(ValueSet valtype, Value* value) {
				m_Values.emplace(valtype, value);
			}
			
			inline Translation* AddTranslation(Type* type, ValueSet valuetype, size_t size) {
				m_Translations.emplace_back(type, valuetype, size);
				return &m_Translations.back();
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
			size_t GetValues(ValueSet valtype, size_t size, std::vector<Value*> & out) const {
				int i = 0;
				AllValues(valtype, [&](Value* value){
					if (value->CanFitSize(size)) {
						out.push_back(value);
						++i;
					}
				});
				return i;
			}
			template<typename TFunc>
			size_t GetValues(ValueSet valtype, size_t size, std::vector<Value*> & out, TFunc func) const {
				int i = 0;
				AllValues(valtype, [&](Value* value){
					if (value->CanFitSize(size) && func(value)) {
						out.push_back(value);
						++i;
					}
				});
				return i;
			}
		
		private:
			void AddValueAttributes(XMLConfig&);
		};
	}
}