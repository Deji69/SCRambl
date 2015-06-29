/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include <iterator>
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
		enum class MatchLevel {
			None, Loose, Basic, Strict
		};

		static ValueSet GetValueTypeByName(std::string name);

		// Internal data types
		class DataType {
			enum Type {
				Int, Float, Fixed, Char, String
			};

			Type m_Type;

		public:
			DataType()
			{ }
			DataType(Type type) : m_Type(type)
			{ }

			inline operator const Type&() const	{ return m_Type; }
			inline operator Type&()	{ return m_Type; }
			inline bool IsInteger() const { return m_Type == Int; }
			inline bool IsFloat() const { return m_Type == Float || m_Type == Fixed; }
			inline bool IsFixed() const { return m_Type == Fixed; }
			inline bool IsChar() const { return m_Type == Char; }
			inline bool IsString() const { return m_Type == String; }

			static bool GetByName(std::string str, Type& dest_type, size_t& dest_size);
		};

		class Value;
		class Basic;
		class Extended;
		class Variable;
		class VariableValue;
		class ArrayValue;

		/* Types::VarType */
		class VarType {
			friend class Types;
			Variable* m_VarType = nullptr;
			Type* m_ValueType = nullptr;

		public:
			VarType() = default;
			VarType(Variable* varType, Type* valType) : m_VarType(varType), m_ValueType(valType)
			{ }

			Type* GetValue() const { return m_ValueType; }
			Variable* GetType() const { return m_VarType; }
			void SetValue(Type* type) { m_ValueType = type; }
			void SetType(Variable* type) { m_VarType = type; }
		};

		/* Types::Type */
		class Type {
			void CopyValues(const Type&);
			void MoveValues(Type&);

		public:
			// TODO: meh
			Type(std::string name, TypeSet type) : m_Name(name), m_Type(type)
			{ }
			Type(const Type& type) : m_Name(type.m_Name), m_Type(type.m_Type) {
				CopyValues(type);
			}
			Type(Type&&);
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
			VariableValue* GetVarValue() const;
			ArrayValue* GetArrayValue() const;

			Type* GetValueType();

			MatchLevel GetMatchLevel(Type* type);

			bool HasValueType(ValueSet type) const;
			template<typename TValue = Value>
			inline std::vector<TValue*> GetValueTypes(ValueSet type) const {
				std::vector<TValue*> vec;
				for (auto val : m_Values) {
					if (val->GetValueType() == type)
						vec.emplace_back(val);
				}
				return vec;
			}
			
			template<typename TValue, typename... TArgs>
			inline TValue* AddValue(TArgs&&... args) {
				auto val = new TValue(std::forward<TArgs>(args)...);
				m_Values.push_back(val);
				return val;
			}

			/*\ Types::Types::Values - Calls the requested function for each matching Value this Type contains \*/
			template<typename TValue, typename TFunc>
			void Values(ValueSet type, TFunc func) const {
				for (auto v : m_Values) {
					if (v->GetValueType() == type && func(static_cast<TValue*>(v)))
						break;
				}
			}

			/*\ Types::Types::AllValues - Calls the requested function for each Value this Type contains \*/
			template<typename TValue, typename TFunc>
			void AllValues(TFunc func) const {
				for (auto v : m_Values) {
					if (func(static_cast<TValue*>(v))) break;
				}
			}

		private:
			std::string m_Name;
			TypeSet m_Type;
			std::vector<Value*> m_Values;
		};

		class Basic : public Type
		{
		public:
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

			Type* GetBasicType() const { return m_BasicType; }
		};

		class Storage {
		public:
			struct Info {
				TypeSet type;
				size_t index;

				Info(TypeSet ty, size_t id) : type(ty), index(id)
				{ }
			};

			using Map = std::unordered_map<std::string, Info>;
			using Vector = std::vector<Info>;

			Storage() { }

			template<typename T>
			void Add(std::string name, Info info) {
				m_Vector.emplace_back(info);
				m_Map.emplace(name, info);
			}
			inline Basic* AddBasic(std::string name) {
				Add<Basic>(name, Info(TypeSet::Basic, m_Basics.size()));
				m_Basics.emplace_back(name);
				return &m_Basics.back();
			}
			inline Extended* AddExtended(std::string name, Basic* basic = nullptr) {
				Add<Extended>(name, Info(TypeSet::Extended, m_Extendeds.size()));
				m_Extendeds.emplace_back(name, basic);
				return &m_Extendeds.back();
			}
			inline Variable* AddVariable(std::string name, XMLValue scope, bool is_array = false) {
				Add<Variable>(name, Info(TypeSet::Variable, m_Variables.size()));
				m_Variables.emplace_back(name, scope, is_array);
				return &m_Variables.back();
			}
			Type* GetType(TypeSet type, size_t id) {
				if (type == TypeSet::Extended)
					return &m_Extendeds[id];
				else if (type == TypeSet::Variable)
					return &m_Variables[id];
				return &m_Basics[id];
			}
			Type* Get(Info info) {
				return GetType(info.type, info.index);
			}
			Type* Get(std::string name) {
				auto it = m_Map.find(name);
				return it != m_Map.end() ? Get(it->second) : nullptr;
			}
			Type* Get(size_t id) {
				return id < m_Vector.size() ? Get(m_Vector[id]) : nullptr;
			}
			size_t GetSize() const {
				return m_Vector.size();
			}
			size_t GetCapacity() const {
				return m_Vector.capacity();
			}
			
			template<typename TFunc>
			size_t Walk(TFunc func) {
				size_t n = 0;
				for (; n < GetSize(); ++n) {
					if (func(Get(n))) break;
				}
				return n;
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

			inline CompatibilityLevel& GetCompatLevel() { return m_Level; }
			inline const CompatibilityLevel& GetCompatLevel() const { return m_Level; }

			inline bool IsFullyCompatible() const { return GetCompatLevel() == Compatible; }
			inline bool IsBasicallyCompatible() const { return GetCompatLevel() != Incompatible; }

			inline operator CompatibilityLevel&() { return GetCompatLevel(); }
			inline operator const CompatibilityLevel&() const { return GetCompatLevel(); }

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
			DataAttributeSet() : AttributeSet(DataAttributeID::None)
			{
				AddAttribute("ID", DataAttributeID::ID);
			}
		};

		template<>
		class DataAttributeSet<DataSourceID::Label> : public AttributeSet<DataAttributeID>
		{
		public:
			DataAttributeSet() : AttributeSet(DataAttributeID::None)
			{
				AddAttribute("Offset", DataAttributeID::Offset);
			}
		};

		template<>
		class DataAttributeSet<DataSourceID::Variable> : public AttributeSet<DataAttributeID>
		{
		public:
			DataAttributeSet() : AttributeSet(DataAttributeID::None)
			{
				AddAttribute("Index", DataAttributeID::Index);
			}
		};

		/*\ Translation \*/
		class Translation
		{
		public:
			using Ref = VecRef<Translation>;
			static const Ref BadRef;

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
					
					void SetValue(XMLValue val) { m_Value = val; }
					XMLValue GetValue() const { return m_Value; }

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
					XMLValue m_Value;
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
			Type* m_Type = nullptr;
			ValueSet m_ValueType = ValueSet::INVALID;
			size_t m_Size = 0;
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
		class Value : public ValueAttributes
		{
			friend Type;

		public:
			Value(Type* type, ValueSet valtype) : m_ValueType(valtype), m_Type(type), m_Size(0)
			{ }
			Value(Type* type, ValueSet valtype, size_t size) : m_ValueType(valtype), m_Type(type), m_Size(size)
			{ }
			inline virtual ~Value() { }

			virtual bool CanFitSize(size_t size) {
				return GetSize() >= size;
			}
			
			inline size_t GetSize() const { return m_Size; }
			inline Type* GetType() const { return m_Type; }
			inline ValueSet GetValueType() const { return m_ValueType; }

			inline Translation::Ref GetTranslation() const { return m_Translation; }
			inline void SetTranslation(Translation::Ref v) { m_Translation = v; }

			template<typename T>
			inline T& Extend() { return *static_cast<T*>(this); }
			template<typename T>
			inline const T& Extend() const { return *static_cast<T*>(this); }

		private:
			ValueSet m_ValueType;
			Type* m_Type;
			size_t m_Size;
			Translation::Ref m_Translation = Translation::BadRef;
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

		class VariableValueAttributes {
			friend class Types;
			Types* m_Types = nullptr;
			
			XMLValue m_Type;
			XMLValue m_Value;

		public:
			VariableValueAttributes(XMLValue type, XMLValue value) : m_Type(type), m_Value(value)
			{ }
			VarType GetVarType() const;
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
		class VariableValue : public Value, public VariableValueAttributes {
		public:
			VariableValue(Type* type, size_t size, XMLValue var, XMLValue val);

			inline bool IsArray() const { return GetValueType() == ValueSet::Array; }
			ArrayValue* ToArray();
			const ArrayValue* ToArray() const;

		protected:
			VariableValue(Type* type, size_t size, XMLValue var, XMLValue val, bool);
		};
		class ArrayValue : public VariableValue {
		public:
			ArrayValue(Type* type, size_t size, XMLValue var, XMLValue val);
			
			inline bool IsArray() const { return true; }
			ArrayValue* ToArray() = delete;
		};

		class Types {
			struct VarValToUpdate {
				VarType& varType;
				bool isArray;
				bool isValue;

				VarValToUpdate(VarType& type, bool isarr, bool isval) : varType(type), isArray(isarr && !isval), isValue(isval)
				{ }
			};

			XMLConfiguration* m_Config;
			Storage m_Types;
			std::multimap<ValueSet, Value*> m_Values;
			std::vector<Translation> m_Translations;
			
			std::unordered_multimap<std::string, VarValToUpdate> m_ValsToUpdate;
			std::unordered_multimap<Type*, Type**> m_TypePointers;

			void UpdatePointer(Type* type, Type* new_ptr) {
				if (type == new_ptr) return;
				auto rg = m_TypePointers.equal_range(type);
				if (rg.first != rg.second) {
					for (auto it = rg.first; it != rg.second; ++it) {
						*it->second = new_ptr;
						m_TypePointers.emplace(new_ptr, it->second);
					}
					m_TypePointers.erase(rg.first, rg.second);
				}
			}
			std::vector<Type*> GetAllTypesVector() {
				std::vector<Type*> vec;
				m_Types.Walk([&vec](Type* type) {
					vec.emplace_back(type);
					return false;
				});
				return vec;
			}
			size_t UpdatePointers(std::vector<Type*> old) {
				size_t i = 0;
				m_Types.Walk([this,&old,&i](Type* type) {
					if (i < old.size()) {
						UpdatePointer(old[i++], type);
						return i >= old.size();
					}
					return true;
				});
				return i;
			}
			
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
				auto old = GetAllTypesVector();
				size_t cap = m_TypePointers.empty() ? 0 : m_Types.GetCapacity();
				auto shared = m_Types.AddBasic(name);
				if (!m_TypePointers.empty() && cap != m_Types.GetCapacity()) UpdatePointers(old);
				return shared;
			}
			inline Extended* AddExtendedType(std::string name, Basic* basic = nullptr) {
				auto old = GetAllTypesVector();
				size_t cap = m_TypePointers.empty() ? 0 : m_Types.GetCapacity();
				auto type = m_Types.AddExtended(name, basic);
				if (!m_TypePointers.empty() && cap != m_Types.GetCapacity()) UpdatePointers(old);
				return type;
			}
			inline Variable* AddVariableType(std::string name, XMLValue scope, bool is_array = false) {
				auto old = GetAllTypesVector();
				size_t cap = m_TypePointers.empty() ? 0 : m_Types.GetCapacity();
				auto type = m_Types.AddVariable(name, scope, is_array);
				if (!m_TypePointers.empty() && cap != m_Types.GetCapacity()) UpdatePointers(old);
				return type;
			}
			inline Type* GetType(std::string name) {
				return m_Types.Get(name);
			}
			inline void AddValue(ValueSet valtype, Value* value) {
				m_Values.emplace(valtype, value);
			}
			
			Translation::Ref AddTranslation(Type* type, ValueSet valuetype, size_t size);

			/*\
			 * Types::Types::AllValues - Calls the requested function for each value of 'valtype'
			 * Returns the number of values found
			\*/
			template<typename TFunc>
			size_t AllValues(ValueSet valtype, TFunc func) const {
				auto rg = m_Values.equal_range(valtype);
				int n = 0;
				for (auto i = rg.first; i != rg.second; ++i) {
					if (func(i->second)) break;
				}
				return n;
			}

			/*\
			 * Types::Types::GetValues - Pushes out a full list of values of 'valtype' which contain values of at least 'size' in bits
			 * Returns the number of values added to the list
			 * Optional bool(Value*) function which should return true if it wants to push the Value
			\*/
			size_t GetValues(ValueSet valtype, size_t size, std::vector<Value*> & out) const {
				int i = 0;
				AllValues(valtype, [&](Value* value){
					if (value->CanFitSize(size)) {
						out.push_back(value);
						++i;
					}
					return false;
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
			void AddValueAttributes(XMLConfig*);
		};
	}
}