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
		Scope(const Scope& v) = delete;
		Scope(Scope&& v) : m_Stuff(std::move(v.m_Stuff))
		{ }
		virtual ~Scope() { }

		Scope& operator=(const Scope& v) = delete;
		Scope& operator=(Scope&& v) {
			if (this != &v) {
				std::swap(m_Stuff, v.m_Stuff);
			}
			return *this;
		}

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
		using List = std::list<ScriptObject>;
		using Map = std::unordered_map<Key, typename List::iterator>;

		ScriptObjects() = default;
		virtual ~ScriptObjects() = default;

		template<typename... TArgs>
		ScriptObject* Add(const Types::Type* type, Key key, TArgs&&... args) {
			bool global = true;
			if (auto varval = type->GetVarValue()) {
				auto vartype = varval->GetVarType();
				global = vartype->IsGlobalVar();
			}
			else if (auto labelval = type->GetLabelValue()) {
				global = labelval->IsGlobal();
			}
			auto& scope = global ? Global() : Local();
			// create object
			auto idx = m_Objects.size();
			m_Objects.emplace_back(scope, type, scope.Size(), key, args...);
			ASSERT(m_Objects.size() > idx);
			// add to scope
			scope.Add(key, m_Objects.back().Ptr());
			// add to global map
			m_Map.emplace(key, std::prev(m_Objects.end()));
			return &m_Objects.back();
		}
		const ScriptObject* Find(Key key) const {
			auto it = m_Map.find(key);
			return it == m_Map.end() ? nullptr : &*it->second;
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
			for (auto it : m_Scopes.back()) {
				// destroy all in it's name
				auto rg = m_Map.equal_range(it.first);
				for (auto it = rg.first; it != rg.second; ++it) {
					m_Objects.erase(it->second);
				}
				m_Map.erase(rg.first, rg.second);
			}
			m_Scopes.erase(m_Scopes.end()-1);
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
		List m_Objects;
		
		ObjectScope m_Global;
		std::vector<ObjectScope> m_Scopes;
	};

	/*\ ScriptObj - A script object \*/
	template<typename TObj, typename TKey>
	class ScriptObject
	{
	public:
		using Scope = Scope<TObj, TKey>;

		template<typename... TArgs>
		ScriptObject(Scope& scope, TArgs&&... args) : m_Object(std::make_unique<TObj>(args...)), m_Scope(scope)
		{ }
		ScriptObject(ScriptObject&& v) : m_Object(std::move(v.m_Object)), m_Scope(std::move(v.m_Scope))
		{ }
		ScriptObject(const ScriptObject&) = delete;
		virtual ~ScriptObject() = default;

		ScriptObject& operator=(const ScriptObject&) = delete;
		ScriptObject& operator=(ScriptObject&& v) {
			if (this != &v) {
				m_Object = std::move(v.m_Object);
				m_Scope = std::move(v.m_Scope);
			}
			return *this;
		}

		inline TObj& Get() const { return *m_Object; }
		inline TObj* Ptr() const { return m_Object.get(); }
		inline TObj& operator*() const { return Ptr(); }
		inline TObj* operator->() const { return Ptr(); }

	private:
		std::unique_ptr<TObj> m_Object;
		Scope& m_Scope;
	};
}