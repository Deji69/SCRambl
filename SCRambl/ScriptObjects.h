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
	/*\
	 * ScriptObj - A script object
	\*/
	template<typename TObj, typename TKey = std::string>
	class ScriptObject
	{
		TObj* m_Object = nullptr;


	public:
		ScriptObject() = default;
		ScriptObject(TObj* obj) : m_Object(obj)
		{ }
		ScriptObject(TObj& obj) : m_Object(&obj)
		{ }
		virtual ~ScriptObject() = default;

		inline TObj& Get() const { return *m_Object; }
		inline TObj* Ptr() const { return m_Object; }

		inline operator bool() const { return Ptr() != nullptr; }
		inline TObj* operator->() const { return Ptr(); }
	};
	
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

		template<typename... TArgs>
		inline ScriptObject Add(const TKey & key, TArgs&&... objargs) {
			auto pair = m_Stuff.emplace(key, ScriptObject(objargs...));
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
		using Object = ScriptObject<TObj, Key>;
		using ObjectScope = Scope<Object, Key>;
		using Pair = std::pair<Object, ObjectScope*>;
		using Map = std::unordered_map<Key, Pair>;

		ScriptObjects() = default;
		virtual ~ScriptObjects() = default;

		template<typename... TArgs>
		Object Add(Key key, TArgs&&... args) {
			m_Objects.emplace_back(args...);
			m_Map.emplace(key, m_Objects.back())
			return m_Objects.back();
		}
		std::pair<Object, ObjectScope*> Find(Key key) {
			static std::pair<Object, ObjectScope*> def(nullptr, nullptr);
			auto it = m_Map.find(key);
			return it == m_Map.end() ? def : it->second;
		}

		ObjectScope* BeginScope() {
			m_Scope.emplace_back();
			return Scope();
		}
		ObjectScope* EndScope() {
			ASSERT(ScopeDepth() > 0);
			m_Scope.erase(m_Scope.end());
			return Scope();
		}
		ObjectScope* Scope() { return ScopeDepth() > 0 ? &m_Scope.back() : nullptr; }
		size_t ScopeDepth() { return m_Scope.size(); }
		
		ObjectScope& Global() { return m_Global; }
		const ObjectScope& Global() const { return m_Global; }

	private:
		Map m_Map;
		std::vector<Object> m_Objects;
		
		ObjectScope m_Global;
		std::vector<ObjectScope> m_Scope;
	};
}