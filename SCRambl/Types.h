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
		class LabelValue;

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

		static ValueSet GetValueTypeByName(std::string);

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

		// Types::VarType
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

		// Types::Type
		class Type {
			void CopyValues(const Type&);
			void MoveValues(Type&&);

		public:
			// TODO: meh
			Type(size_t id, std::string name, TypeSet type) : m_ID(id), m_Name(name), m_Type(type)
			{ }
			Type(const Type& type) : m_ID(type.m_ID), m_Name(type.m_Name), m_Type(type.m_Type) {
				CopyValues(type);
			}
			Type(Type&&);
			inline virtual ~Type() { }

			bool IsGlobalVar() const;
			bool IsScopedVar() const;
			size_t GetVarMinIndex() const;
			size_t GetVarMaxIndex() const;

			size_t GetID() const;
			std::string GetName() const;
			TypeSet GetType() const;
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
			LabelValue* GetLabelValue() const;
			const Type* GetValueType() const;

			MatchLevel GetMatchLevel(const Type*) const;

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
			// Calls the requested function for each matching Value this Type contains
			template<typename TValue, typename TFunc>
			void Values(ValueSet type, TFunc func) const {
				for (auto v : m_Values) {
					if (v->GetValueType() == type && func(static_cast<TValue*>(v)))
						break;
				}
			}
			// Calls the requested function for each Value this Type contains
			template<typename TValue, typename TFunc>
			void AllValues(TFunc func) const {
				for (auto v : m_Values) {
					if (func(static_cast<TValue*>(v))) break;
				}
			}

		private:
			size_t m_ID;
			std::string m_Name;
			TypeSet m_Type;
			std::vector<Value*> m_Values;
		};

		class Basic : public Type {
		public:
			Basic(size_t id, std::string name) : Type(id, name, TypeSet::Basic)
			{ }
		
		protected:
			Basic(size_t id, std::string name, bool) : Type(id, name, TypeSet::Extended)
			{ }
		};
		class Variable : public Type {
			XMLValue m_Scope;
			XMLValue m_IsArray;
			XMLValue m_Size;

			// <MinIndex>
			XMLValue m_MinIndex = 1;
			// <MaxIndex>
			XMLValue m_MaxIndex = LONG_MAX;

		public:
			Variable(size_t id, std::string name, XMLValue scope, XMLValue isarray, size_t size) : Type(id, name, TypeSet::Variable),
				m_Scope(scope), m_IsArray(isarray), m_Size(size)
			{ }

			XMLValue Scope() const { return m_Scope; }
			bool IsGlobal() const { return lengthcompare(m_Scope.AsString("global"), "local") == 0; }
			XMLValue IsArray() const { return m_IsArray; }
			XMLValue MinIndex() const { return m_MinIndex; }
			XMLValue MaxIndex() const { return m_MinIndex; }
			XMLValue Size() const { return m_Size; }

			void SetMinIndex(XMLValue v) { m_MinIndex = v; }
			void SetMaxIndex(XMLValue v) { m_MaxIndex = v; }
		};
		class Extended : public Type {
			Type* m_BasicType;

		public:
			Extended(size_t id, std::string name, Type* basic = nullptr) : Type(id, name, TypeSet::Extended),
				m_BasicType(basic)
			{ }

			Type* GetBasicType() const { return m_BasicType; }
		};

		class ITypeRef {
		public:
			virtual ~ITypeRef() = default;
			virtual TypeSet GetType() const = 0;
			virtual Type& Get() const = 0;
			virtual bool OK() const = 0;
			inline operator bool() const { return OK(); }
			inline bool IsBasic() const { return GetType() == TypeSet::Basic; }
			inline bool IsVariable() const { return GetType() == TypeSet::Variable; }
			inline bool IsExtended() const { return GetType() == TypeSet::Extended; }
			inline Basic& AsBasic() { return static_cast<Basic&>(Get()); }
			inline const Basic& AsBasic() const { return static_cast<const Basic&>(Get()); }
			inline Extended& AsExtended() { return static_cast<Extended&>(Get()); }
			inline const Extended& AsExtended() const { return static_cast<const Extended&>(Get()); }
			inline Variable& AsVariable() { return static_cast<Variable&>(Get()); }
			inline const Variable& AsVariable() const { return static_cast<const Variable&>(Get()); }
		};

		template<typename T>
		class TypeRef : public ITypeRef {
		public:
			TypeRef() = default;
			TypeRef(std::nullptr_t) : TypeRef() { }
			TypeRef(TypeRef<T>&& v) : m_Type(std::move(v.m_Type))
			{ }
			TypeRef(VecRef<T> v) : m_Type(v)
			{ }
			virtual TypeSet GetType() const override { return Get().GetType(); }
			virtual Type& Get() const override { return *m_Type; }
			virtual bool OK() const override { return m_Type.OK(); }
			VecRef<T> Ref() const { return m_Type; }

		private:
			VecRef<T> m_Type;
		};

		using BasicRef = TypeRef<Basic>;
		using ExtendedRef = TypeRef<Extended>;
		using VariableRef = TypeRef<Variable>;

		class Storage {
		public:
			using Vector = std::vector<std::unique_ptr<ITypeRef>>;
			using Map = std::unordered_map<std::string, size_t>;

			Storage() { }

			inline BasicRef& AddBasic(std::string name) {
				auto& ref = Add<Basic>(name, { m_Basics });
				m_Basics.emplace_back(ref.Ref().Index(), name);
				return ref;
			}
			inline ExtendedRef& AddExtended(std::string name, Basic* basic = nullptr) {
				auto& ref = Add<Extended>(name, { m_Extendeds });
				m_Extendeds.emplace_back(ref.Ref().Index(), name, basic);
				return ref;
			}
			inline VariableRef& AddVariable(std::string name, XMLValue scope, bool is_array = false, size_t size = 32) {
				auto& ref = Add<Variable>(name, { m_Variables });
				m_Variables.emplace_back(ref.Ref().Index(), name, scope, is_array, size);
				return ref;
			}
			size_t GetTypeID(TypeSet type, size_t id) {
				if (type == TypeSet::Extended && id < m_Extendeds.size())
					return m_Extendeds[id].GetID();
				else if (type == TypeSet::Variable && id < m_Variables.size())
					return m_Variables[id].GetID();
				else if (type == TypeSet::Basic && id < m_Basics.size())
					return m_Basics[id].GetID();
				else BREAK();
				return -1;
			}
			template<typename T = Type>
			inline TypeRef<T> Get(size_t id) { return static_cast<TypeRef<T>&>(*m_Vector[id]); }
			template<typename T = Type>
			inline TypeRef<const T> Get(size_t id) const { return static_cast<TypeRef<const T>&>(*m_Vector[id]); }
			template<typename T = Type>
			inline T* Get(TypeSet type, size_t id) { return Get(GetTypeID(type)); }
			template<typename T = Type>
			inline TypeRef<const T> Get(TypeSet type, size_t id) const { return Get(GetTypeID(type)); }
			template<typename T = Type>
			inline TypeRef<T> Get(std::string name) {
				auto it = m_Map.find(name);
				return it != m_Map.end() ? Get<T>(it->second) : nullptr;
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
					if (func(Get(n).Ref().Ptr())) break;
				}
				return n;
			}

		protected:
			template<typename T>
			TypeRef<T>& Add(std::string name, TypeRef<T> ref) {
				m_Vector.emplace_back(std::make_unique<TypeRef<T>>(ref));
				m_Map.emplace(name, m_Vector.size() - 1);
				return static_cast<TypeRef<T>&>(*m_Vector.back());
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
			None, Env, Value, Number, Text, Command, Variable, Label, Condition,
		};
		enum class DataAttributeID {
			None, Value, Size, Offset,
			// Command
			ID, Name, NumArgs,
			// Condition
			IsNOT,
			// Variable
			Index, IsArray, IsGlobal
		};
		
		class DataSource {
		public:
			static std::string GetNameByID(DataSourceID);
			static DataSourceID GetIDByName(std::string);

			DataSource() = default;
			DataSource(DataSourceID id) : m_ID(id), m_Name(GetNameByID(id))
			{ }
			DataSource(std::string name) : m_ID(GetIDByName(name)), m_Name(name)
			{ }

			DataSourceID ID() const { return m_ID; }
			std::string Name() const { return m_Name; }

		private:
			DataSourceID m_ID = DataSourceID::None;
			std::string m_Name;
		};
		class DataAttribute {
		public:
			static std::string GetNameByID(DataAttributeID);
			static DataAttributeID GetIDByName(std::string);

			DataAttribute() = default;
			DataAttribute(DataAttributeID id) : m_ID(id), m_Name(GetNameByID(m_ID))
			{ }
			DataAttribute(std::string name) : m_ID(GetIDByName(name)), m_Name(name)
			{ }

			DataAttributeID ID() const { return m_ID; }
			std::string Name() const { return m_Name; }

		private:
			DataAttributeID m_ID = DataAttributeID::None;
			std::string m_Name;
		};

		class DataSourceSet : public AttributeSet<DataSourceID> {
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
				AddAttribute("Index", DataAttributeID::Index);
				AddAttribute("Value", DataAttributeID::Value);
				AddAttribute("Size", DataAttributeID::Size);
				AddAttribute("Offset", DataAttributeID::Offset);
				AddAttribute("Name", DataAttributeID::Name);
				AddAttribute("IsNOT", DataAttributeID::IsNOT);
			}
		};

		class Xlation;

		// Translation
		class Translation {
		public:
			class Data {
				static const size_t c_invalid_size;

			public:
				class Field
				{
				public:
					Field(DataType type, DataSourceID src, DataAttributeID attr) :
						m_Type(type), m_Source(src), m_Attribute(attr), m_Size(type == DataType::Char ? 8 : 0), m_SizeLimit(false)
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

			Translation(VecRef<Type> type, ValueSet valuetype, size_t size) : m_Type(type), m_ValueType(valuetype), m_Size(size)
			{ }

			Data& AddData() {
				m_Data.emplace_back(m_Size);
				return m_Data.back();
			}
			DataRef GetData(size_t i) { return {m_Data, i}; }
			size_t GetDataCount() const { return m_Data.size(); }

			size_t GetSize(Xlation);

		private:
			VecRef<Type> m_Type = nullptr;
			ValueSet m_ValueType = ValueSet::INVALID;
			size_t m_Size = 0;
			std::vector<Data> m_Data;
		};

		using DataAttributes = Attributes<DataAttributeID, DataAttributeSet>;
		using DataAttributesMap = std::map<DataSourceID, DataAttributes>;
		using DataAttributesFunc = std::function<XMLValue(DataSourceID, DataAttributeID)>;

		// The Xlation's Will Convert You!
		class Xlation {
		public:
			Xlation() = default;
			Xlation(Translation::Ref translation, DataAttributesFunc&& func) : m_Func(func), m_Translation(translation)
			{ }
			virtual ~Xlation() = default;

			Translation::Ref GetTranslation() const { return m_Translation; }
			void SetAttribute(DataSourceID src, DataAttributeID attr, XMLValue val) {
				m_AttributesMap[src].SetAttribute(attr, val);
			}
			void SetAttributes(DataSourceID src, DataAttributes attributes) { m_AttributesMap[src] = attributes; }
			XMLValue GetAttribute(DataSourceID src, DataAttributeID attr) const {
				auto it = m_AttributesMap.find(src);
				return it != m_AttributesMap.end() ? it->second.GetAttribute(attr) : m_Func(src, attr);
			}
			void SetTranslation(Translation::Ref ref) { m_Translation = ref; }

			Xlation MergeAttributes(const Xlation& v) const {
				Xlation r = *this;
				for (auto attr : v.m_AttributesMap) {
					r.m_AttributesMap.emplace(attr.first, attr.second);
				}
				return r;
			}

		protected:
			const DataAttributes& GetAttributes(DataSourceID id) const { return m_AttributesMap.at(id); }

		private:
			static const DataAttributesMap s_NullMap;

			Translation::Ref m_Translation;
			DataAttributesFunc m_Func;
			DataAttributesMap m_AttributesMap;
		};

		/*\ Value of Type for translation \*/
		class Value {
			friend Type;

		public:
			Value(VecRef<Type> type, ValueSet valtype) : m_ValueType(valtype), m_Type(type), m_Size(-1)
			{ }
			Value(VecRef<Type> type, ValueSet valtype, size_t size) : m_ValueType(valtype), m_Type(type), m_Size(size)
			{ }
			inline virtual ~Value() { }

			virtual bool CanFitSize(size_t size) {
				return GetSize() >= size;
			}
			
			inline size_t GetSize() const { return m_Size != -1 ? m_Size : 0; }
			inline const Type* GetType() const { return m_Type.Ptr(); }
			inline ValueSet GetValueType() const { return m_ValueType; }

			inline Translation::Ref GetTranslation() const { return m_Translation; }
			inline void SetTranslation(Translation::Ref v) { m_Translation = v; }

			template<typename T>
			inline T& Extend() { return *static_cast<T*>(this); }
			template<typename T>
			inline const T& Extend() const { return *static_cast<T*>(this); }

		private:
			ValueSet m_ValueType;
			VecRef<Type> m_Type;
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
			NumberValue(VecRef<Type> type, NumberValueType numtype, size_t size) : Value(type, ValueSet::Number, size), m_Type(numtype)
			{ }

			inline NumberValueType	GetNumberType() const { return m_Type; }

		private:
			NumberValueType	m_Type;
		};
		class TextValue : public Value {
		public:
			TextValue(VecRef<Type> type, size_t size, XMLValue mode = "", XMLValue terminate = "") : Value(type, ValueSet::Text, size),
				m_Mode(mode), m_Terminate(terminate)
			{ }

		private:
			XMLValue m_Mode;
			XMLValue m_Terminate;
		};
		class LabelValue : public Value
		{
		public:
			LabelValue(VecRef<Type> type, size_t size, XMLValue scope = "") : Value(type, ValueSet::Label, size), m_Scope(scope)
			{ }

			bool IsGlobal() const { return lengthcompare(m_Scope.AsString("global"), "local") == 0; }

		private:
			XMLValue m_Scope;
		};
		class VariableValue : public Value {
			friend Types;
		public:
			VariableValue(VecRef<Type> type, size_t size, VecRef<Variable> vartype, VecRef<Type> valtype);

			inline bool IsScoped() const { return m_VarType->IsScopedVar(); }
			inline bool IsGlobal() const { return !IsScoped(); }
			inline bool IsArray() const { return GetValueType() == ValueSet::Array; }
			const Variable* GetVarType() const { return m_VarType.Ptr(); }
			const Type* GetValType() const { return m_ValType.Ptr(); }
			ArrayValue* ToArray();
			const ArrayValue* ToArray() const;

		protected:
			VariableValue(VecRef<Type> type, size_t size, VecRef<Variable> var, VecRef<Type> val, bool);

		private:
			VecRef<Variable> m_VarType;
			VecRef<Type> m_ValType;
		};
		class ArrayValue : public VariableValue {
		public:
			ArrayValue(VecRef<Type> type, size_t size, VecRef<Variable> var, VecRef<Type> val);
			
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

			inline Basic* AddType(std::string name) {
				auto old = GetAllTypesVector();
				size_t cap = m_TypePointers.empty() ? 0 : m_Types.GetCapacity();
				auto ref = m_Types.AddBasic(name);
				if (!m_TypePointers.empty() && cap != m_Types.GetCapacity()) UpdatePointers(old);
				return &ref.AsBasic();
			}
			inline Extended* AddExtendedType(std::string name, Basic* basic = nullptr) {
				auto old = GetAllTypesVector();
				size_t cap = m_TypePointers.empty() ? 0 : m_Types.GetCapacity();
				auto type = m_Types.AddExtended(name, basic);
				if (!m_TypePointers.empty() && cap != m_Types.GetCapacity()) UpdatePointers(old);
				return &type.AsExtended();
			}
			inline Variable* AddVariableType(std::string name, XMLValue scope, bool is_array = false, size_t size = 32) {
				auto old = GetAllTypesVector();
				size_t cap = m_TypePointers.empty() ? 0 : m_Types.GetCapacity();
				auto type = m_Types.AddVariable(name, scope, is_array, size);
				if (!m_TypePointers.empty() && cap != m_Types.GetCapacity()) UpdatePointers(old);
				return &type.AsVariable();
			}
			template<typename T = Type>
			inline TypeRef<T> GetType(std::string name) { return m_Types.Get<T>(name); }
			inline void AddValue(ValueSet valtype, Value* value) {
				m_Values.emplace(valtype, value);
			}
			
			Translation::Ref AddTranslation(TypeRef<Type> type, ValueSet valuetype, size_t size);

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
				size_t i = 0;
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
				size_t i = 0;
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