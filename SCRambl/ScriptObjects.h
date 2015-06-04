/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <unordered_map>
#include "Types.h"

namespace SCRambl
{
	template<typename TObj, typename TKey = std::string>
	class ScriptObject;
	
	/*\
	 * Scope - Scope of variables, labels, you name it
	\*/
	template<typename TObj, typename TKey = std::string>
	class Scope
	{
	public:
		using Map = std::unordered_map<TKey, TObj*>;

	private:
		Map m_Stuff;

	public:
		Scope() = default;
		virtual ~Scope() { }

		inline void Add(const TKey& key, TObj* obj) {
			m_Stuff.emplace(key, obj);
		}

		inline typename Map::iterator Begin() {
			return m_Stuff.begin();
		}
		inline typename Map::iterator End() {
			return m_Stuff.end();
		}
		inline typename Map::const_iterator Begin() const {
			return m_Stuff.begin();
		}
		inline typename Map::const_iterator End() const {
			return m_Stuff.end();
		}
		inline typename Map::iterator begin() { return Begin(); }
		inline typename Map::iterator end() { return End(); }
		inline typename Map::const_iterator begin() const { return Begin(); }
		inline typename Map::const_iterator end() const { return End(); }

		TObj* Find(const TKey & key) const {
			auto it = m_Stuff.find(key);
			return it == m_Stuff.end() ? nullptr : it->second;
		}
	};

	/*\
	 * ScriptObjects - Manager for script objects
	\*/
	template<typename TObj, typename TKey = std::string>
	class ScriptObjects
	{
	public:
		using Key = TKey;
		using Object = TObj;
		using ScriptObject = ScriptObject<Object, Key>;
		using ObjectScope = Scope<Object, Key>;
		using Map = std::unordered_map<Key, ScriptObject*>;

		ScriptObjects() = default;
		virtual ~ScriptObjects() = default;

		template<typename... TArgs>
		ScriptObject* Add(Types::Type* type, Key key, TArgs&&... args) {
			// create object, add to scope
			m_Objects.emplace_back(type->IsGlobal() ? Global() : Local(), type, key, args...);
			auto& obj = m_Objects.back();
			// add to scope
			(type->IsGlobal() ? Global() : Local()).Add(key, obj.Ptr());
			// add to global map
			m_Map.emplace(key, &obj);
			return &obj;
		}
		ScriptObject* Find(Key key) {
			auto it = m_Map.find(key);
			return it == m_Map.end() ? nullptr : it->second;
		}

		size_t LocalDepth() const { return m_Scopes.size(); }
		bool HasLocal() const { return !m_Scopes.empty(); }
		
		const ObjectScope& Global() const {
			return m_Global;
		}
		const ObjectScope& Local() const {
			ASSERT(HasLocal());
			return m_Scopes.back();
		}
		const ObjectScope& Scope() const {
			return HasLocal() ? Local() : Global();
		}

		const ObjectScope& BeginLocal() {
			m_Scopes.emplace_back();
			return Scope();
		}
		const ObjectScope& EndLocal() {
			ASSERT(LocalDepth() > 0);
			m_Scopes.erase(m_Scopes.end());
			return Scope();
		}

	private:
		ObjectScope& Global() {
			return m_Global;
		}
		ObjectScope& Local() {
			ASSERT(HasLocal());
			return m_Scopes.back();
		}

		Map m_Map;
		std::vector<ScriptObject> m_Objects;
		
		ObjectScope m_Global;
		std::vector<ObjectScope> m_Scopes;
	};

	/*\
	 * ScriptObj - A script object
	\*/
	template<typename TObj, typename TKey>
	class ScriptObject
	{
	public:
		using Scope = Scope<TObj, TKey>;

		template<typename... TArgs>
		ScriptObject(const Scope& scope, TArgs&&... args) : m_Object(args...), m_Scope(scope)
		{ }
		virtual ~ScriptObject() = default;

		inline TObj& Get() { return m_Object; }
		inline const TObj& Get() const { return m_Object; }

		inline TObj* Ptr() { return &m_Object; }
		inline const TObj* Ptr() const { return &m_Object; }

		inline const Scope& GetScope() const { return m_Scope; }

		inline TObj& operator*() const { return Ptr(); }
		inline TObj* operator->() const { return Ptr(); }

	private:
		TObj m_Object;
		const Scope& m_Scope;
	};
}