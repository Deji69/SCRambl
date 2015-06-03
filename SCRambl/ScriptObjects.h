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
		using ScriptObject = ScriptObject<TObj, TKey>;

		struct Item {
			ScriptObject Object;
			bool IsInScope;
		};

		using Vector = std::vector<ScriptObject>;

		using Iterator = typename Vector::iterator;
		using ReverseIterator = typename Vector::reverse_iterator;
		using ConstIterator = typename Vector::const_iterator;
		using ConstReverseIterator = typename Vector::const_reverse_iterator;
		using Map = std::unordered_map<TKey, Iterator>;

	private:
		Map m_Stuff;
		Vector m_Storage;

	public:
		Scope() = default;
		virtual ~Scope() { }

		inline ScriptObject Add(const TKey& key, TObj* obj) {
			auto pair = m_Stuff.emplace(key, objargs);
			return pair.second ? *pair.first->second : nullptr;
		}

		inline Iterator Begin() {
			return m_Stuff.begin();
		}
		inline Iterator End() {
			return m_Stuff.end();
		}
		inline ConstIterator Begin() const {
			return m_Stuff.begin();
		}
		inline ConstIterator End() const {
			return m_Stuff.end();
		}
		inline Iterator begin() { return Begin(); }
		inline Iterator end() { return End(); }
		inline ConstIterator begin() const { return Begin(); }
		inline ConstIterator end() const { return End(); }

		TObj Find(const TKey & key) const {
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
		using Pair = std::pair<Object, ObjectScope*>;
		using Map = std::unordered_map<Key, Pair>;

		ScriptObjects() = default;
		virtual ~ScriptObjects() = default;

		template<typename... TArgs>
		ScriptObject Add(Key key, bool global = false, TArgs&&... args) {
			// create object
			m_Objects.emplace_back(args...);
			auto& obj = m_Objects.back();
			// add to scope
			if (global) m_Global.Add(key, obj);
			else m_Scope.back().Add(key, obj);
			// add to global map
			m_Map.emplace(key, std::make_pair<Pair>(obj, global ? &m_Global : &m_Scope.back()));
			return obj;
		}
		Object Find(Key key) {
			auto it = m_Map.find(key);
			return it == m_Map.end() ? Object(nullptr) : Object(it->second.first);
		}

		const ObjectScope* BeginScope() {
			m_Scope.emplace_back();
			return Scope();
		}
		const ObjectScope* EndScope() {
			ASSERT(ScopeDepth() > 0);
			m_Scope.erase(m_Scope.end());
			return Scope();
		}
		const ObjectScope* Scope() const { return ScopeDepth() > 0 ? &m_Scope.back() : nullptr; }
		size_t ScopeDepth() { return m_Scope.size(); }
		
		ObjectScope& Global() { return m_Global; }
		const ObjectScope& Global() const { return m_Global; }

	private:
		Map m_Map;
		std::vector<ScriptObject> m_Objects;
		
		ObjectScope m_Global;
		std::vector<ObjectScope> m_Scope;
	};

	/*\
	 * ScriptObj - A script object
	\*/
	template<typename TObj, typename TKey>
	class ScriptObject
	{
		TObj m_Object;
		Scope<TObj, TKey>* m_Scope = nullptr;

	public:
		ScriptObject() = default;
		template<typename... TArgs>
		ScriptObject(TArgs&&... args) : m_Object(args...)
		{ }
		template<typename... TArgs>
		ScriptObject(Scope<TObj, TKey>* scope, TArgs&&... args) : m_Object(args...), m_Scope(scope)
		{ }
		virtual ~ScriptObject() = default;

		inline TObj& Get() { return m_Object; }
		inline const TObj& Get() const { return m_Object; }

		inline TObj* Ptr() { return &m_Object; }
		inline const TObj* Ptr() const { return &m_Object; }

		inline Scope<TObj, TKey>* GetScope() const { return m_Scope; }

		inline TObj* operator->() const { return Ptr(); }
	};
}