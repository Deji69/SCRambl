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
		/*\ Tokens::Token - Types of token information \*/
		enum class Type : char {
			None, Directive,
			Command, CommandDecl, CommandCall, CommandOverload, Variable,
			Identifier, Label, LabelRef, Number, Operator, String, Character, Delimiter
		};

		/*\ Tokens::Symbol - Base for parsed token data \*/
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
	
		/*\ Tokens::Token - Script token wrapper \*/
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
				if (m_Token) delete m_Token;
				m_Token = tok;
			}
			
			inline IToken* GetToken() { return m_Token; }
			inline const IToken* GetToken() const { return GetToken(); }

			inline operator IToken*() { return GetToken(); }
			inline operator const IToken*() const { return GetToken(); }
		};

		using Iterator = VecIndex<Token>;

		/*\ Tokens::Storage - Token container \*/
		class Storage {
		public:
			typedef std::vector<Token> Vector;

		private:
			Vector m_Tokens;

		public:
			/*class Iterator {
				Vector::iterator m_It;
				size_t m_Index = 0;

			public:
				//using Vector = std::vector<Iterator>;
				using CVector = std::vector<Iterator>;

				Iterator() = default;
				Iterator(Vector& cont) : m_It(cont.begin()), m_Index(0)
				{ }
				Iterator(Vector::iterator it, const Vector& cont) : m_It(it), m_Index(it - cont.begin())
				{ }

				inline Token* Get() const {
					return &*m_It;
				}
				inline size_t GetIndex() const {
					return m_Index;
				}

				// Access
				inline Token* operator*() const {
					return Get();
				}
				inline Token* operator->() const {
					return Get();
				}
				inline Token* operator[](size_t n) const {
					return &m_It[n];
				}

				// Manipulation
				inline Iterator& operator++() {
					++m_It;
					++m_Index;
					return *this;
				}
				inline Iterator operator++(int) {
					auto it = *this;
					++m_It;
					++m_Index;
					return it;
				}
				inline Iterator& operator--() {
					--m_It;
					--m_Index;
					return *this;
				}
				inline Iterator operator--(int) {
					auto it = *this;
					--m_It;
					--m_Index;
					return it;
				}
				inline Iterator& operator+=(size_t n) {
					m_It += n;
					m_Index += n;
					return *this;
				}
				inline Iterator& operator-=(size_t n) {
					m_It -= n;
					m_Index -= n;
					return *this;
				}
				inline Iterator operator+(size_t n) {
					auto it = *this;
					it.m_It += n;
					it.m_Index += n;
					return it;
				}
				inline Iterator operator-(size_t n) {
					auto it = *this;
					it.m_It -= n;
					it.m_Index -= n;
					return it;
				}

				// Comparison
				inline bool operator==(const Iterator& rhs) const {
					return m_It == rhs.m_It;
				}
				inline bool operator!=(const Iterator& rhs) const {
					return !(*this == rhs);
				}
				inline bool operator<(const Iterator& rhs) const {
					return m_Index < rhs.m_Index;
				}
				inline bool operator<=(const Iterator& rhs) const {
					return m_Index <= rhs.m_Index;
				}
				inline bool operator>(const Iterator& rhs) const {
					return m_Index > rhs.m_Index;
				}
				inline bool operator>=(const Iterator& rhs) const {
					return m_Index >= rhs.m_Index;
				}
			};
			-/*/
			Storage() = default;
			Storage(const Vector& vec) = delete;
			~Storage() {
				for (auto& tok : m_Tokens) {
					if (tok.GetToken()) delete tok.GetToken();
				}
			}

			/* Manipulation */

			// Insert
			template<typename TToken, typename... TArgs>
			inline Token Add(Scripts::Position pos, TArgs&&... args) {
				m_Tokens.emplace_back(pos, new TToken(args...));
				return m_Tokens.back();
			}

			/* Navigation */

			// Begin
			inline Iterator Begin() { return{ m_Tokens, m_Tokens.begin() }; }
			// End
			inline Iterator End() { return{ m_Tokens, m_Tokens.end() }; }

			// STL hates my style
			inline Iterator begin() { return Begin(); }
			inline Iterator end() { return End(); }

			/* Access */
			inline const Token Front() const { return m_Tokens.front(); }
			inline const Token Back() const { return m_Tokens.back(); }
			inline Token Front() { return m_Tokens.front(); }
			inline Token Back() { return m_Tokens.back(); }

			/* Info */
			inline size_t Size() const { return m_Tokens.size(); }
			inline bool Empty() const { return m_Tokens.empty(); }
		};
		
		class Line {
		private:
			typedef std::vector<Token*> Vector;
			Vector m_Line;

		public:
			using Iter = Vector::iterator;
			using RevIter = Vector::reverse_iterator;
			using CIter = Vector::const_iterator;
			using CRevIter = Vector::const_reverse_iterator;
			using Ref = Vector::reference;
			using CRef = Vector::reference;

			Line() = default;

			void AddToken(Token* tok) {
				auto dest_it = m_Line.end();
				auto dest_col = -1;
				for (auto it = m_Line.begin(); it != m_Line.end(); ++it) {
					auto ptr = *it;
					auto col = ptr->GetPosition().GetColumn();

					if (dest_it == m_Line.end() || col < dest_col) {
						dest_it = it;
						dest_col = col;
					}
					else if (col >= dest_col) {
						if (col == dest_col) dest_col = -1;
						break;
					}
				}

				if (dest_it != m_Line.end()) {
					if (dest_col == -1) return;
					m_Line.emplace(dest_it, tok);
				}
				else m_Line.emplace_back(tok);
			}
			Token* GetToken(int col) {
				Token* r;
				for (auto ptr : m_Line) {
					if (ptr->GetPosition().GetColumn() >= col)
						return r;
					r = ptr;
				}
				return r;
			}

			inline Iter begin() { return m_Line.begin(); }
			inline Iter end() { return m_Line.end(); }
			inline CIter cbegin() { return m_Line.cbegin(); }
			inline CIter cend() { return m_Line.cend(); }
		};
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