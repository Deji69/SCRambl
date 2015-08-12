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
	
	/*\ Scope - Scope of variables, labels, you name it \*/
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
		inline size_t Size() const { return m_Stuff.size(); }

		TObj* Find(const TKey & key) const {
			auto it = m_Stuff.find(key);
			return it == m_Stuff.end() ? nullptr : it->second;
		}
	};

	/*\ ScriptObjects - Manager for script objects \*/
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
			auto& scope = type->IsGlobal() ? Global() : Local();
			// create object
			m_Objects.emplace_back(scope, scope.Size(), type, key, args...);
			auto& obj = m_Objects.back();
			// add to scope
			scope.Add(key, obj.Ptr());
			// add to position map
			//m_PosMap.emplace(pos, &obj);
			// add to global map
			m_Map.emplace(key, &obj);
			// add to object map
			m_ObjectMap.emplace(obj.Ptr(), &obj);
			return &obj;
		}
		/*std::pair<ScriptObject*, ScriptObject*> FindClosest(Scripts::Position pos) {
			auto pair = std::make_pair(nullptr, nullptr);
			auto it = m_PosMap.lower_bound(pos);
			if (it != map.end()) {
				pair.first = it->second;
				if (it!= map.begin()) {
					--it;
					pair.second = it->second;
				}
			}
			return pair;
		}*/
		ScriptObject* Find(Key key) const {
			auto it = m_Map.find(key);
			return it == m_Map.end() ? nullptr : it->second;
		}
		ScriptObject* Find(const Object* obj) const {
			auto it = m_ObjectMap.find(obj);
			return it == m_ObjectMap.end() ? nullptr : it->second;
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
		std::map<const Object*, ScriptObject*> m_ObjectMap;
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
		ScriptObject(const Scope& scope, size_t index, TArgs&&... args) : m_Object(args...), m_Scope(scope), m_Index(index)
		{ }
		virtual ~ScriptObject() = default;

		inline size_t Index() const { return m_Index; }

		inline TObj& Get() { return m_Object; }
		inline const TObj& Get() const { return m_Object; }

		inline TObj* Ptr() { return &m_Object; }
		inline const TObj* Ptr() const { return &m_Object; }

		inline TObj& operator*() const { return Ptr(); }
		inline TObj* operator->() const { return Ptr(); }

	private:
		size_t m_Index;
		TObj m_Object;
		const Scope& m_Scope;
	};
}