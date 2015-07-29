//**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "TokenInfo.h"
#include "Scripts-code.h"

namespace SCRambl
{
	namespace Tokens
	{
		using Iterator = VecIndex<class Token>;

		// Tokens::Token - Types of token information
		enum class Type : char {
			None, Directive,
			Command, CommandDecl, CommandOverload, Variable,
			Identifier, Label, LabelRef, Number, Operator, String, Character, Delimiter,

			// parse tokens
			ArgList, CommandCall,
		};
		
		// Base for parsed token data
		class Symbol {
		public:
			Symbol(Type type) : m_Type(type)
			{ }
			inline virtual ~Symbol()
			{ }

			inline Type GetType() const { return m_Type; }

			template<typename  T>
			inline T& Extend() { return static_cast<T&>(*this); }
			template<typename  T>
			inline const T& Extend() const { return static_cast<T&>(*this); }

		private:
			Type m_Type;
		};
	
		// Script token wrapper
		class Token {
			Scripts::Position m_Position;
			IToken*	m_Token = nullptr;

		public:
			Token(Scripts::Position pos, IToken* tok) :
				m_Position(pos),
				m_Token(tok)
			{ }
			Token(const Token* ptr) : Token(*ptr)
			{ }

			inline Scripts::Position& GetPosition() { return m_Position; }
			inline const Scripts::Position& GetPosition() const { return GetPosition(); }

			template<typename T>
			inline T* GetToken() { return static_cast<T*>(m_Token); }
			template<typename T>
			inline const T* GetToken() const { return static_cast<T*>(m_Token); }

			template<typename T>
			inline void SetToken(T tok) {
				m_Token = tok;
			}
			
			inline IToken* GetToken() { return m_Token; }
			inline const IToken* GetToken() const { return GetToken(); }

			inline operator IToken*() { return GetToken(); }
			inline operator const IToken*() const { return GetToken(); }
			inline IToken* operator->() { return GetToken(); }
			inline const IToken* operator->() const { return GetToken(); }
		};

		// Token container
		class Storage {
		public:
			typedef std::vector<Token> Vector;

		private:
			Vector m_Tokens;

		public:
			Storage() = default;
			Storage(const Vector& vec) = delete;
			~Storage() {
				for (auto& tok : m_Tokens) {
					if (tok.GetToken()) delete tok.GetToken();
				}
			}

			// Manipulation //
			template<typename TToken, typename... TArgs>
			inline Token Add(Scripts::Position pos, TArgs&&... args) {
				m_Tokens.emplace_back(pos, new TToken(args...));
				return m_Tokens.back();
			}
			// Navigation //
			inline Iterator Begin() { return{ m_Tokens, m_Tokens.begin() }; }
			inline Iterator End() { return{ m_Tokens, m_Tokens.end() }; }
			inline Iterator begin() { return Begin(); }
			inline Iterator end() { return End(); }
			// Access //
			inline const Token Front() const { return m_Tokens.front(); }
			inline const Token Back() const { return m_Tokens.back(); }
			inline Token Front() { return m_Tokens.front(); }
			inline Token Back() { return m_Tokens.back(); }
			// Info //
			inline size_t Size() const { return m_Tokens.size(); }
			inline bool Empty() const { return m_Tokens.empty(); }
		};
		// Token line vector
		class Line {
		private:
			using Vector = std::vector<Token*>;
			template<typename T>
			using VectorIndex = VecIndex<Token*, size_t, Vector, T>;

		public:
			using Iter = VectorIndex<Vector::iterator>;
			using RevIter = VectorIndex<Vector::reverse_iterator>;
			using CIter = VectorIndex<Vector::const_iterator>;
			using CRevIter = VectorIndex<Vector::const_reverse_iterator>;
			using Ref = VecRef<Token*>;
			using CRef = VecRef<const Token*>;

			Line() = default;

			void AddToken(Token*);
			Ref GetToken(size_t col);

			inline Iter begin() { return{ m_Line.begin(), m_Line }; }
			inline Iter end() { return{ m_Line.end(), m_Line }; }
			inline CIter cbegin() { return{ m_Line.cbegin(), m_Line }; }
			inline CIter cend() { return{ m_Line.cend(), m_Line }; }

		private:
			Vector m_Line;
		};
		// Token map
		class Map {
			typedef std::map<long long, Line> TokenList;

			TokenList m_Map;

		public:
			using Iter = TokenList::iterator;
			using RevIter = TokenList::reverse_iterator;
			using CIter = TokenList::const_iterator;
			using CRevIter = TokenList::const_reverse_iterator;
			using Ref = TokenList::reference;
			using CRef = TokenList::reference;
			using RefType = TokenList::referent_type;

			Map() = default;

			Line* AddLine(long long);
			Line* GetLine(long long);
		};
	}
}