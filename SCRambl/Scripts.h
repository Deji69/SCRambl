/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <assert.h>
#include <unordered_map>
#include "utils.h"
#include "Scripts-code.h"
//#include "FileSystem.h"
#include "Tokens.h"
#include "Symbols.h"
#include "TokenInfo.h"
#include "Labels.h"
#include "Types.h"
#include "Variables.h"
#include "ScriptObjects.h"

namespace SCRambl
{
	using TokenSymbol = SCRambl::Tokens::Symbol;
	using TokenPtrVec = std::vector<IToken*>;
	using SymbolPtrVec = std::vector<TokenSymbol*>;

	namespace Scripts
	{
		using CodeList = std::list<Line>;
		using CodeListRef = CodeList::iterator;
		using Files = std::vector<File>;
		using FileRef = VecRef<File>;

		/*\ Scripts::File - Script files and includes
		\*/
		class File
		{
		public:
			File();
			File(const File&);
			File(File*);
			File(Code*);
			File(std::string);
			File(File*, std::string);
			File(Code*, std::string);
			~File();

			bool IsOpen() const;
			bool IsInclude() const;
			long GetNumLines() const;
			long GetNumIncludes() const;
			std::string GetPath() const;
			Code* GetCode() const;
			void SetCode(Code*);
			bool Open(std::string);
			bool Open(Code*, std::string);
			FileRef IncludeFile(Position&, std::string);

		private:
			void ReadFile(std::ifstream&);

			bool m_FileOpen = false;
			File* m_Parent;
			Code* m_Code;
			Position m_Begin;
			Position m_End;
			Files m_Includes;
			std::string m_Path;
			long m_NumLines = 0;
		};
		
		/*\ Scripts::Token - Script token wrapper \*/
		class Token
		{
			Position m_Position;
			IToken*	m_Token = nullptr;
			TokenSymbol* m_Symbol = nullptr;

		public:
			Token(Position pos, IToken* tok) :
				m_Position(pos),
				m_Token(tok)
			{ }
			Token(const Token* ptr) : Token(*ptr)
			{ }
			Token(TokenSymbol* symb) : m_Symbol(symb)
			{ }

			inline Position& GetPosition() { return m_Position; }
			inline const Position& GetPosition() const { return GetPosition(); }
			
			template<typename T>
			inline T* GetToken() { return static_cast<T*>(m_Token); }
			template<typename T>
			inline const T* GetToken() const { return static_cast<T*>(m_Token); }

			template<typename T>
			inline void SetToken(T tok) {
				if (m_Token) delete m_Token;
				m_Token = tok;
			}
			template<typename T>
			inline void SetSymbol(T sym) {
				if (m_Symbol) delete m_Symbol;
				m_Symbol = sym;
			}
			
			inline IToken* GetToken() { return m_Token; }
			inline const IToken* GetToken() const { return GetToken(); }
			inline TokenSymbol*& GetSymbol() { return m_Symbol; }
			inline const TokenSymbol*& GetSymbol() const { return GetSymbol(); }

			inline operator IToken*() { return GetToken(); }
			inline operator const IToken*() const { return GetToken(); }
			inline operator TokenSymbol*&()	{ return GetSymbol(); }
			inline operator const TokenSymbol*&() const	{ return GetSymbol(); }
		};

		/*\ Scripts::Tokens - Script token container \*/
		class Tokens
		{
		public:
			typedef std::vector<Token> Vector;

		private:
			Vector m_Tokens;

		public:
			/*\
			 * Scripts::Tokens::Iterator - All this just to pair together an index with it
			\*/
			class Iterator
			{
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

			Tokens() = default;
			Tokens(const Vector& vec) = delete;
			~Tokens() {
				for (auto& tok : m_Tokens) {
					if (tok.GetToken()) delete tok.GetToken();
				}
			}

			/* Manipulation */

			// Insert
			template<typename TToken, typename... TArgs>
			inline Token Add(Position pos, TArgs&&... args) {
				m_Tokens.emplace_back(pos, new TToken(args...));
				return m_Tokens.back();
			}

			/* Navigation */

			// Begin
			inline Iterator Begin() { return{ m_Tokens.begin(), m_Tokens }; }
			// End
			inline Iterator End() { return{ m_Tokens.end(), m_Tokens }; }

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
		class TokenLine
		{
		private:
			typedef std::vector<Token*> Line;
			Line m_Line;

		public:
			using Iter = Line::iterator;
			using RevIter = Line::reverse_iterator;
			using CIter = Line::const_iterator;
			using CRevIter = Line::const_reverse_iterator;
			using Ref = Line::reference;
			using CRef = Line::reference;

			TokenLine() = default;

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
		class TokenMap
		{
			typedef std::map<long long, TokenLine> TokenList;

			TokenList m_Map;

		public:
			using Iter = TokenList::iterator;
			using RevIter = TokenList::reverse_iterator;
			using CIter = TokenList::const_iterator;
			using CRevIter = TokenList::const_reverse_iterator;
			using Ref = TokenList::reference;
			using CRef = TokenList::reference;
			using RefType = TokenList::referent_type;

			TokenMap() = default;

			TokenLine* AddLine(long long);
			TokenLine* GetLine(long long);
		};

		using Labels = Scope<Label>;

		/*class LScript
		{
		public:
			LScript();

			inline Labels& GetLabels()							{ return m_Labels; }
			inline const Labels& GetLabels() const				{ return m_Labels; }
			inline Variables& GetVariables()					{ return m_Variables; }
			inline const Variables& GetVariables() const		{ return m_Variables; }

		private:
			Labels			m_Labels;
			Variables		m_Variables;
		};

		typedef std::vector<LScript> LScripts;*/
	}

	class Script
	{
	public:
		Script();

		// Open file 
		Scripts::FileRef OpenFile(const std::string&);
		
		// Must be \0 terminated
		void SetCode(const void *);
		
		// Is there a file for the main code?
		bool IsFileOpen() const;
		
		// Returns number of loaded code lines
		long long GetNumLines() const;

		//void LoadFile(const std::string&);
		void ReadFile(std::ifstream&, Scripts::Code&);

		/* Include File
		[in] position
		[in] include file path */
		Scripts::Position Include(Scripts::Position&, const std::string&);

		Scripts::TokenMap GenerateTokenMap();

		// Get number of (additional) local scripts
		//size_t NumLScripts() const { return m_LScripts.size(); }

		// Get the code list
		Scripts::Code& GetCode() { return *m_Code; }

		// Get the labels
		Scripts::Labels& GetLabels() { return m_Labels ; }

		// Get the tokens
		Scripts::Tokens& GetTokens() { return m_Tokens; }

	private:
		bool ProcessCodeLine(const std::string&, CodeLine&, bool = false);

		Scripts::Tokens m_Tokens;
		Scripts::Files m_Files;
		//Scripts::LScripts m_LScripts;
		Scripts::Labels m_Labels;
		
		//Scripts::LScript m_LScriptMain;
		//Scripts::LScript* m_LScript;
		Scripts::FileRef m_File;
		Scripts::Code* m_Code;
	};
}