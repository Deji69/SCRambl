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

namespace SCRambl
{
	typedef SCRambl::Tokens::Symbol TokenSymbol;

	namespace Scripts
	{
		typedef std::list<Line> CodeList;
		typedef std::vector<std::shared_ptr<File>> Files;

		/*\
		 * Scripts::File - Script files and includes
		\*/
		class File
		{
		public:
			using Shared = std::shared_ptr<File>;
			
			File();
			File(File*);
			File(Code::Shared);
			File(std::string);
			File(File*, std::string);
			File(Code::Shared, std::string);

			bool IsOpen() const;
			bool IsInclude() const;
			long GetNumLines() const;
			long GetNumIncludes() const;
			Code::Shared GetCode() const;
			void SetCode(Code::Shared);
			bool Open(std::string);
			bool Open(Code::Shared, std::string);
			File::Shared IncludeFile(Position&, std::string);

		private:
			void ReadFile(std::ifstream&);

			bool m_FileOpen = false;
			File* m_Parent = nullptr;
			Code::Shared m_Code;
			Position m_Begin;
			Position m_End;
			Files m_Includes;
			std::string m_Path;
			long m_NumLines = 0;
		};
		
		/*\
		 * Scripts::Token - Script token wrapper
		\*/
		class Token
		{
			Position						m_Position;
			IToken::Shared					m_Token = nullptr;
			std::shared_ptr<TokenSymbol>	m_Symbol = nullptr;

		public:
			typedef std::shared_ptr<Token> Shared;

			Token(Position pos, IToken::Shared tok) :
				m_Position(pos),
				m_Token(tok)
			{ }
			Token(Token::Shared shared) : Token(*shared)
			{ }
			Token(std::shared_ptr<TokenSymbol> symb) : m_Symbol(symb)
			{ }

			inline Position & GetPosition()					{ return m_Position; }
			inline const Position & GetPosition() const		{ return GetPosition(); }
			
			template<typename T>
			inline std::shared_ptr<T> GetToken()							{ return std::static_pointer_cast<T>(m_Token); }
			template<typename T>
			inline std::shared_ptr<const T> GetToken() const				{ return std::static_pointer_cast<T>(m_Token); }

			template<typename T>
			inline void SetToken(const T& tok)								{ m_Token = /*std::static_pointer_cast<IToken>(tok)*/ tok; }
			
			inline IToken::Shared GetToken()								{ return m_Token; }
			inline const IToken::Shared GetToken() const					{ return GetToken(); }
			inline std::shared_ptr<TokenSymbol>& GetSymbol()				{ return m_Symbol; }
			inline const std::shared_ptr<TokenSymbol>& GetSymbol() const	{ return GetSymbol(); }

			inline operator IToken::Shared()							{ return GetToken(); }
			inline operator const IToken::Shared() const				{ return GetToken(); }
			inline operator std::shared_ptr<TokenSymbol>&()				{ return GetSymbol(); }
			inline operator const std::shared_ptr<TokenSymbol>&() const	{ return GetSymbol(); }
		};

		/*\
		 * Scripts::Tokens - Script token container
		\*/
		class Tokens
		{
		public:
			typedef std::vector<Token::Shared> Vector;

		private:
			Vector			m_Tokens;

		public:
			/*\
			 * Scripts::Tokens::Iterator - All this just to pair together an index with it
			\*/
			class Iterator
			{
				Vector::iterator		m_It;
				size_t					m_Index = 0;

			public:
				//using Vector = std::vector < Iterator > ;
				using CVector = std::vector < Iterator >;

				Iterator() = default;
				Iterator(Vector & cont) : m_It(cont.begin()), m_Index(0)
				{ }
				Iterator(Vector::iterator it, const Vector & cont) : m_It(it), m_Index(it - cont.begin())
				{ }

				inline Token::Shared Get() const {
					return *m_It;
				}
				inline size_t GetIndex() const {
					return m_Index;
				}

				// Access
				inline Token::Shared operator*() {
					return Get();
				}
				inline const Token::Shared operator*() const {
					return Get();
				}
				inline const Token::Shared operator->() const {
					return **this;
				}
				inline Token::Shared operator[](size_t n) const {
					return m_It[n];
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
					return m_It < rhs.m_It;
				}
				inline bool operator<=(const Iterator& rhs) const {
					return m_It <= rhs.m_It;
				}
				inline bool operator>(const Iterator& rhs) const {
					return m_It > rhs.m_It;
				}
				inline bool operator>=(const Iterator& rhs) const {
					return m_It >= rhs.m_It;
				}
			};

			Tokens() = default;
			Tokens(const Vector & vec) : m_Tokens(vec) { }

			/* Manipulation */

			// Insert
			template<typename TToken, typename... TArgs>
			inline Token::Shared Add(Position pos, TArgs... args)		{
				auto ptr = std::make_shared<Token>(pos, std::make_shared<TToken>(std::forward<TArgs>(args)...));
				m_Tokens.emplace_back(ptr);
				return ptr;
			}

			/* Navigation */

			// Begin
			inline Iterator Begin()					{ return{ m_Tokens.begin(), m_Tokens }; }
			// End
			inline Iterator End()					{ return{ m_Tokens.end(), m_Tokens }; }

			// STL hates my style
			inline Iterator begin()					{ return Begin(); }
			inline Iterator end()					{ return End(); }

			/* Access */
			inline const Token::Shared Front() const	{ return m_Tokens.front(); }
			inline Token::Shared Front()				{ return m_Tokens.front(); }
			inline const Token::Shared Back() const		{ return m_Tokens.back(); }
			inline Token::Shared Back()					{ return m_Tokens.back(); }

			/* Info */
			inline size_t Size() const				{ return m_Tokens.size(); }
			inline bool Empty() const				{ return m_Tokens.empty(); }

			//
			template<typename TToken, typename... TArgs>
			static inline Token::Shared MakeShared(Position pos, TArgs... args) {
				return std::make_shared<Token>(pos, std::make_shared<TToken>(std::forward<TArgs>(args)...));
			}
		};
		class TokenLine
		{
		private:
			typedef std::vector<Token::Shared> Line;
			Line		m_Line;

		public:
			using Iter = Line::iterator;
			using RevIter = Line::reverse_iterator;
			using CIter = Line::const_iterator;
			using CRevIter = Line::const_reverse_iterator;
			using Ref = Line::reference;
			using CRef = Line::reference;
			using Shared = std::shared_ptr < TokenLine > ;

			TokenLine() = default;

			void AddToken(Token::Shared tok) {
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
			Token::Shared GetToken(int col) {
				Token::Shared r;
				for (auto ptr : m_Line) {
					if (ptr->GetPosition().GetColumn() >= col)
						return r;
					r = ptr;
				}
				return r;
			}

			inline Iter begin()						{ return m_Line.begin(); }
			inline Iter end()						{ return m_Line.end(); }
			inline CIter cbegin()					{ return m_Line.cbegin(); }
			inline CIter cend()						{ return m_Line.cend(); }
		};
		class TokenMap
		{
			
			typedef std::map<int, TokenLine::Shared> TokenList;

			TokenList			m_Map;

		public:
			using Iter = TokenList::iterator;
			using RevIter = TokenList::reverse_iterator;
			using CIter = TokenList::const_iterator;
			using CRevIter = TokenList::const_reverse_iterator;
			using Ref = TokenList::reference;
			using CRef = TokenList::reference;
			using RefType = TokenList::referent_type;
			using Shared = std::shared_ptr < TokenMap > ;

			TokenMap() = default;

			TokenLine::Shared AddLine(int);
			TokenLine::Shared GetLine(int);
		};

		/*\
		 * Scripts::Scope - Scope of variables, labels, you name it
		\*/
		template<typename TObj, typename TKey = std::string, typename TCont = std::unordered_map<TKey, TObj>>
		class Scope
		{
		public:
			using Container = TCont;
			using Iterator = typename Container::iterator;
			using ReverseIterator = typename Container::reverse_iterator;
			using ConstIterator = typename Container::reverse_iterator;

		private:
			Container			m_Stuff;

		public:
			Scope() = default;
			inline virtual ~Scope() { }

			template<typename... TArgs>
			inline void Insert(const TKey & key, TArgs&&... obj) {
				m_Stuff.emplace(key, std::forward<TArgs>(obj)...);
			}

			inline Iterator Begin() {
				return m_Stuff.begin();
			}
			inline Iterator End() {
				return m_Stuff.end();
			}
			inline Iterator begin()				{ return Begin(); }
			inline Iterator end()				{ return End(); }

			inline TObj Find(const TKey & key) {
				auto it = m_Stuff.find(key);
				return it == m_Stuff.end() ? nullptr : it->second;
			}
		};

		using Labels = Scope<std::shared_ptr<Label>>;
		using Variables = Scope<std::shared_ptr<Variable>>;

		//template<typename T> typedef Variable<T> Variable;
		//template<typename T> typedef Scope<Variable<T>::Shared> Variables;

		class LScript
		{
		public:
			using Shared = std::shared_ptr<LScript>;

			LScript();

			inline Labels& GetLabels()							{ return m_Labels; }
			inline const Labels& GetLabels() const				{ return m_Labels; }
			inline Variables& GetVariables()					{ return m_Variables; }
			inline const Variables& GetVariables() const		{ return m_Variables; }

		private:
			Labels			m_Labels;
			Variables		m_Variables;
		};

		typedef std::vector<LScript> LScripts;
	}

	class Script
	{
	public:
		Script();

		// Open file 
		Scripts::File::Shared OpenFile(const std::string&);
		
		// Must be \0 terminated
		void SetCode(const void *);
		
		// Is there a file for the main code?
		bool IsFileOpen() const;
		
		// Returns number of loaded code lines
		long long GetNumLines() const;

		//void LoadFile(const std::string&);
		void ReadFile(std::ifstream &, Scripts::Code &);

		/* Include File
		[in] position
		[in] include file path */
		Scripts::Position Include(Scripts::Position &, const std::string &);

		Scripts::TokenMap::Shared GenerateTokenMap();

		// Get number of (additional) local scripts
		size_t NumLScripts() const { return m_LScripts.size(); }

		// Get the code list
		Scripts::Code& GetCode() { return *m_Code; }

		// Get the labels
		Scripts::Labels& GetLabels() { return m_LScript ? m_LScript->GetLabels() : m_Labels ; }

		// Get the variables
		Scripts::Variables& GetVariables() { return m_Variables; }
		const Scripts::Variables& GetVariables() const { return m_Variables; }

		// Get the current local script
		Scripts::LScript* GetLScript() { return m_LScript; }
		const Scripts::LScript* GetLScript() const { return m_LScript; }

		// Get one of the local scripts
		Scripts::LScript* GetLScript(size_t i) { return &m_LScripts[i]; }
		const Scripts::LScript* GetLScript(size_t i) const { return &m_LScripts[i]; }

		// Get the tokens
		Scripts::Tokens& GetTokens() { return m_Tokens; }

		// X
		inline std::vector<std::shared_ptr<TokenSymbol>> & GetDeclarations() { return m_Declarations; }
		inline const std::vector<std::shared_ptr<TokenSymbol>> & GetDeclarations() const { return m_Declarations; }

	private:
		bool ProcessCodeLine(const std::string&, CodeLine&, bool = false);

		Scripts::Tokens m_Tokens;
		Scripts::Files m_Files;
		Scripts::LScripts m_LScripts;
		Scripts::Labels m_Labels;
		Scripts::Variables m_Variables;
		
		Scripts::LScript m_LScriptMain;
		Scripts::LScript* m_LScript;
		Scripts::File::Shared m_File;
		Scripts::Code::Shared m_Code;

		std::vector<std::shared_ptr<TokenSymbol>> m_Declarations;
	};
}