/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include <iterator>
#include <map>
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
		class Value;
		class Basic;
		class Extended;
		class Variable;
		class VariableValue;
		class ArrayValue;

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
		public:
			enum Type {
				Int, Float, Fixed, Char, String, Args, INVALID
			};

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

			static Type GetByName(std::string str);
			static bool GetByName(std::string str, Type& dest_type, size_t& dest_size);

		private:
			Type m_Type;
		};

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
			None, Value, Number, Text, Command, Variable, Label, Condition,
		};
		enum class DataAttributeID {
			None, Value, Size, Offset,
			// Command
			ID, Name, NumArgs,
			// Condition
			IsNOT
		};

		class DataSourceSet : public AttributeSet<DataSourceID>
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
				AddAttribute("Condition", DataSourceID::Condition);
			}
		};
		class DataAttributeSet : public AttributeSet<DataAttributeID> {
		public:
			DataAttributeSet() : AttributeSet(DataAttributeID::None) {
				AddAttribute("ID", DataAttributeID::ID);
				AddAttribute("Index", DataAttributeID::ID);
				AddAttribute("Value", DataAttributeID::Value);
				AddAttribute("Size", DataAttributeID::Size);
				AddAttribute("Offset", DataAttributeID::Offset);
				AddAttribute("Name", DataAttributeID::Name);
				AddAttribute("IsNOT", DataAttributeID::IsNOT);
			}
		};

		class Xlation;

		/*\ Translation \*/
		class Translation
		{
		public:
			class Data {
				static const size_t c_invalid_size;

			public:
				class Field
				{
				public:
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
					DataType m_Type = DataType::INVALID;
					DataSourceID m_Source = DataSourceID::None;
					DataAttributeID m_Attribute = DataAttributeID::None;
					bool m_SizeLimit = false;
					size_t m_Size = 0;
					XMLValue m_Value;
				};

				Data(size_t& size) : m_TranslationSize(size)
				{ }

				Field* AddField(DataType type, DataSourceID src, DataAttributeID attr) {
					if (type.IsChar()) m_TranslationSize += sizeof(uint8_t);
					else if (type.IsFloat()) m_TranslationSize += sizeof(float);
					//else BREAK();	// error: gotta have a size
					m_Fields.emplace_back(type, src, attr);
					return &m_Fields.back();
				}
				Field* AddField(DataType type, DataSourceID src, DataAttributeID attr, size_t size) {
					if (type.IsInteger()) m_TranslationSize += size;
					else if (type.IsFloat()) m_TranslationSize += size;
					else if (type.IsFixed()) m_TranslationSize += size;
					else if (type.IsString()) m_TranslationSize += size * sizeof(uint8_t);
					else BREAK();   // error: doesn't support size
					m_Fields.emplace_back(type, src, attr, size);
					return &m_Fields.back();
				}
				size_t GetNumFields() const { return m_Fields.size(); }
				VecRef<Field> GetField(size_t i) { return {m_Fields, i}; }
				
			private:
				size_t& m_TranslationSize;
				std::vector<Field> m_Fields;
			};

			using Ref = VecRef<Translation>;
			using DataRef = VecRef<Data>;
			using FieldRef = VecRef<Data::Field>;
			static const Ref BadRef;

			Translation(Type* type, ValueSet valuetype, size_t size) : m_Type(type), m_ValueType(valuetype), m_Size(size)
			{ }

			Data& AddData() {
				m_Data.emplace_back(m_Size);
				return m_Data.back();
			}
			DataRef GetData(size_t i) { return {m_Data, i}; }
			size_t GetDataCount() const { return m_Data.size(); }

		private:
			Type* m_Type = nullptr;
			ValueSet m_ValueType = ValueSet::INVALID;
			size_t m_Size = 0;
			std::vector<Data> m_Data;
		};

		/*\ The Xlation's Will Convert You! \*/
		class Xlation
		{
		public:
			using Attributes = Attributes<DataAttributeID, DataAttributeSet>;

			Xlation() = default;
			Xlation(Translation::Ref translation) : m_Translation(translation)
			{ }

			Translation::Ref GetTranslation() const { return m_Translation; }
			void SetAttribute(DataSourceID src, DataAttributeID attr, XMLValue val) { m_AttributesMap[src].SetAttribute(attr, val); }
			void SetAttributes(DataSourceID src, Attributes attributes) { m_AttributesMap[src] = attributes; }
			XMLValue GetAttribute(DataSourceID src, DataAttributeID attr) const { return m_AttributesMap.at(src).GetAttribute(attr); }
			const Attributes& GetAttributes(DataSourceID id) const { return m_AttributesMap.at(id); }

		private:
			Translation::Ref m_Translation;
			std::map<DataSourceID, Attributes> m_AttributesMap;
		};

		/*\ Value of Type for translation \*/
		class Value
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

		class NumberValue : public Value
		{
		public:
			NumberValue(Type* type, NumberValueType numtype, size_t size) : Value(type, ValueSet::Number, size), m_Type(numtype)
			{ }

			inline NumberValueType	GetNumberType() const { return m_Type; }

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
		class VariableValue : public Value {
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
				static const DataAttributeSet s_set;
				return s_set.GetAttribute(name);
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
			template<typename T = Value, typename TFunc>
			size_t AllValues(ValueSet valtype, TFunc func) const {
				auto rg = m_Values.equal_range(valtype);
				int n = 0;
				for (auto i = rg.first; i != rg.second; ++i) {
					if (func(&i->second->Extend<T>())) break;
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