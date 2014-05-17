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
#include <unordered_map>
#include "utils.h"

namespace SCRambl
{
	// TODO: Clean-up EVERYTHING. It works pretty nicely tho...

	namespace Lexer
	{
		namespace Expressions
		{
			/*template<char match, char... rest>
			class Equality
			{
				std::unordered_map<std::string, bool> Search;

			public:
				Equality(match, rest)
				{
				}
			};*/

			class Equality
			{
				std::unordered_map<std::string, bool>		Haystack;

			public:
				Equality & AddMatch(const std::string & match)
				{
					Haystack.insert({ match, true });
					return *this;
				}

				bool IsMatch(const std::string & needle) const
				{
					return Haystack.find(needle) != Haystack.end();
				}
			};
			
			template <class ExpressionType>
			class Expression
			{
				ExpressionType		m_Expression;

			public:
				bool IsMatch(const std::string & search)
				{
					return m_Expression.IsMatch(search);
				}
			};
		}
		namespace Grammar
		{
			namespace Punctuation
			{
			}

			class Whitespace
			{
			public:
				enum State { before, during, after };

			private:
				State		m_State;

			public:

				template<typename CharType>
				bool operator ()(CharType a) const
				{
					switch (a)
					{
					case ' ':
					case '\n':
					case '\r':
					case '\t':
					case '\v':
					case '\f':
						return true;
					default:
					}
				}
			};

			template<class Scanner>
			class Lexeme
			{
			public:
				enum State { before, during, after };

			private:
				State		m_State;
				Scanner	&	m_Scanner;

			public:
				Lexeme(Scanner & scanner) : m_Scanner(scanner)
				{
				}

				bool operator ()(char ch) const
				{
					/*switch (m_State)
					{
					case before:
						if (m_StartsWith(ch)) m_State = during;
						break;
					case during:
						if (m_ConsistsOf(ch)) return true;
						else m_State = after;
					case after:
						if (m_EndsWith(ch)) return true;
						else return false;
					default:
						throw(std::runtime_error("bad state"));
					}*/
					return false;
				}
			};

			/*template<typename before_fun, typename during_fun = before_fun, typename after_fun = before_fun>
			class Lexeme
			{
				enum State { before, during, after };

			public:
				State		m_State;
				before_fun	m_StartsWith;
				during_fun	m_ConsistsOf;
				after_fun	m_EndsWith;

			public:
				Lexeme(before_fun starts, during_fun consists, after_fun ends) :
					m_StartsWith(starts), m_ConsistsOf(consists), m_EndsWith(ends)
				{
				}

				bool operator ()(char ch) const
				{
					switch (m_State)
					{
					case before:
						if (m_StartsWith(ch)) m_State = during;
						break;
					case during:
						if (m_ConsistsOf(ch)) return true;
						else m_State = after;
					case after:
						if (m_EndsWith(ch)) return true;
						else return false;
					default:
						throw(std::runtime_error("bad state"));
					}
					return false;
				}
			};*/

			template<typename lexeme_type>
			class Scanner
			{
				std::vector<char> Characters;

			public:
				Scanner(const std::initializer_list<char> & chars) : Characters(chars)
				{
				}

				void AddCharacter(char character)
				{
					Characters.push_back(character);
				}

				bool operator()(lexeme_type character) const
				{

				}
			};
		}

		class RegExp
		{
			//std::vector<Expressions::Expression> m_Expressions;

		public:
			RegExp()
			{
			}
		};

		template<class TokenType>
		class Token
		{
		private:
			TokenType			m_TokenType;
			std::string			m_TokenString;

		public:
			Token(TokenType type, std::string str) : m_TokenType(type), m_TokenString(str)
			{
			}

			inline TokenType GetType() const	{ return m_TokenType; }
			inline operator TokenType() const	{ return GetType(); }
			inline operator std::string &()		{ return m_TokenString; }
			inline operator const std::string &() const { return m_TokenString; }
		};

		/*class LexerState
		{
		public:
			enum State	{ before, inside, after };

		private:
			State							m_State = before;
			const char					*	m_Code = nullptr;
			size_t							m_Index = 0;

		public:
			LexerState() = default;
			LexerState(const char * szStr) : m_State(before), m_Code(szStr), m_Index(0)
			{
			}
			LexerState(const std::string & str) : m_State(before), m_Code(str.c_str()), m_Index(0)
			{
			}

			inline operator const char *() const{ return m_Code ? &m_Code[m_Index] : m_Code; }
			inline operator State &()			{ return m_State; }
			inline operator State () const		{ return m_State; }

			/*inline LexerState & operator=(const std::string & str)
			{
				m_Code = str.c_str();
				m_Index = 0;
			}*\/

			inline LexerState & operator++()
			{
				if (m_Code[m_Index]) ++m_Index;
				return *this;
			}
			inline LexerState & operator+=(int nVal)
			{
				for (; nVal-- && m_Code[m_Index]; ++m_Index);
				return *this;
			}
			inline LexerState & operator=(State val)
			{
				m_State = val;
				return *this;
			}
		};*/

		class LexerState
		{
		public:
			enum State	{ before, inside, after };

		private:
			State							m_State = before;

		public:
			LexerState() = default;
			LexerState(State state) : m_State(state)
			{}

			inline operator State &()			{ return m_State; }
			inline operator State () const		{ return m_State; }
		};

		class Scanner
		{
		public:
			virtual bool Scan(const LexerState & state, const std::string & str, std::string::const_iterator & it)
			{
				if (it != str.end()) ++it;
				return false;
			}
		};

		template <class Scanner>
		class Lexeme
		{
		private:
			const char	*	m_pStart = nullptr;
			const char	*	m_pEnd = nullptr;

			Scanner			m_Scanner;

		public:
			//Lexeme() = default;
			Lexeme(Scanner obj) : m_pStart(nullptr), m_pEnd(nullptr), m_Scanner(obj)
			{
			}
			virtual ~Lexeme()
			{
				delete m_Scanner;
			}

			// Return true if state can advance
			bool Lex(LexerState state, const std::string & str, std::string::const_iterator & it)
			{
				// The scanner should return the number of characters that have been matched
				// The state will be advanced on the first nonzero value
				int result = 0;

				m_pEnd = &*it;

				switch (state)
				{
				case LexerState::before:
					if (!(result = m_Scanner->Scan(state, &*it)))
						return false;
					m_pStart = &*it;
					it += result;
					break;
				case LexerState::inside:
					if (!(result = m_Scanner->Scan(state, &*it)))
						return false;
					it += result;
					break;
				case LexerState::after:
					if (!(result = m_Scanner->Scan(state, &*it)))
						return false;
					it += result;
					break;
				default: throw(std::runtime_error("Invalid LexerState"));
				}

				return true;
			}

			inline const char *		Start() const		{ return m_pStart; }
			inline const char *		End() const			{ return m_pEnd; }
		};

		template<typename TokenIDType>
		class LexerToken
		{
			TokenIDType						m_Type;
			std::string						m_Token;

		public:
			LexerToken(TokenIDType type, std::string::const_iterator start, std::string::const_iterator end) : m_Type(type), m_Token(start, end)
			{}

			inline TokenIDType				GetType() const			{ return m_Type; }
			inline std::string				GetToken() const		{ return m_Token; }

			inline operator TokenIDType &()							{ return m_Type; }
			inline operator std::string &()							{ return m_Token; }
			inline operator TokenIDType () const					{ return m_Type; }
			inline operator const std::string &() const				{ return m_Token; }
		};

		template<typename TokenIDType, class Scanner = Scanner>
		class LexerScanner
		{
			TokenIDType						m_Type;
			LexerState					&	m_State;
			Scanner						&	m_Scanner;
			const std::string			&	m_String;
			std::string::const_iterator	&	m_Current;

		public:
			LexerScanner(LexerState & state, Scanner & scanner, const std::string & str, TokenIDType type) : m_State(state), m_Scanner(scanner), m_String(str), m_Type(type)
			{
				m_Current = str.begin();
			}

			void Scan()
			{
				if (m_Current != m_String.end())
				{
					switch (m_State)
					{
					case LexerState::before:
						scanner.Scan(m_State, m_String, m_Current);
						break;
					}
				}
			}
		};

		template<typename TokenIDType>
		class Lexer
		{
			using ScannerList = std::vector<std::pair<TokenIDType, Scanner&>>;

			LexerState						m_State;
			std::string::const_iterator		m_Start;
			std::string::const_iterator		m_Current;
			const std::string			*	m_pString;
			
			ScannerList						m_Scanners;
			typename ScannerList::iterator	m_ScannerIter;
			Scanner						*	m_Scanner = nullptr;

		public:
			inline LexerState			&	GetState()			{ return m_State; }
			inline const LexerState		&	GetState() const	{ return m_State; }
			inline TokenIDType				GetTokenType() const{ return m_ScannerIter[0].first; }
			inline LexerToken<TokenIDType>	Tokenize() const	{ return{ m_ScannerIter[0].first, m_Start, m_Current }; }

			virtual ~Lexer()
			{
			}

			template<class Scanner>
			void AddTokenScanner(TokenIDType tokenid, Scanner & scanner)
			{
				m_Scanners.emplace_back(tokenid, scanner);
			}

			// Return true if state can advance
			bool Scan(const std::string & str, std::string::const_iterator & it)
			{
				// The scanner should return the number of characters that have been matched
				// The state will be advanced on the first nonzero value
				//auto it = it_in;
				m_pString = &str;

				if (m_State == LexerState::before)
				{
					m_Start = it;
					m_Current = it;

					for (m_ScannerIter = m_Scanners.begin(); m_ScannerIter != m_Scanners.end(); ++m_ScannerIter)
					{
						m_Scanner = &m_ScannerIter[0].second;

						if (m_Scanner->Scan(m_State, str, m_Current))
						{
							m_State = LexerState::inside;
							it = m_Current;
							return true;
						}
						else m_Current = m_Start;
					}
				}
				else throw(std::runtime_error("Invalid Lexer State"));
				return false;
			}
			bool Scan(std::string::const_iterator & it)
			{
				auto & str = *m_pString;
				switch (m_State)
				{
				case LexerState::inside:
					if (!m_Scanner->Scan(m_State, str, m_Current))
					{
						m_State = LexerState::after;
					}
					break;

				case LexerState::after:
					if (m_Scanner->Scan(m_State, str, m_Current))
					{
						m_State = LexerState::before;
						it = m_Current;
						break;
					}
					else
					{
						m_Current = m_Start;
						for (m_ScannerIter; m_ScannerIter != m_Scanners.end(); ++m_ScannerIter)
						{
							m_Scanner = &m_ScannerIter[0].second;

							if (m_Scanner->Scan(m_State, str, m_Current))
							{
								m_State = LexerState::inside;
								break;
							}
							else m_Current = m_Start;
						}
						m_Current = m_Start;
						it = m_Current;
						m_State = LexerState::before;
						return false;
					}
					break;
				default: throw(std::runtime_error("Invalid Lexer State"));
				}

				it = m_Current;
				return true;
			}
		};

		/*template<typename TokenType>
		class Lexer
		{
			LexerState				m_State;
			const std::string	*	m_String = nullptr;
			std::string::const_iterator	m_Current;
			std::string::const_iterator	m_TokStart;
			std::string::const_iterator	m_TokEnd;

			using Token = std::tuple<TokenType, std::string::const_iterator, std::string::const_iterator>;
			
			TokenType				m_Type;
			Token		m_Token;

			using ExpressionList = std::vector< std::pair<TokenType, Lexeme<Scanner*>*> >;
			//typedef std::vector< std::pair<TokenType, Lexeme<Scanner*>*> > ExpressionList;
			
			ExpressionList				m_Expressions;
			typename ExpressionList::iterator	m_ActiveExpression = m_Expressions.end();		// (wtf)?

		public:
			//enum State	{ before, inside, after };

			Lexer() = default;

			virtual ~Lexer()
			{
				for (auto & p : m_Expressions)
				{
					delete p.second;
				}
			}

			template<class ScannerObj>
			Lexer & AddLexeme(TokenType type)
			{
				m_Expressions.emplace_back(type, new Lexeme<Scanner*>(new ScannerObj()));
				return *this;
			}

			Lexer & Lex(const std::string & str)
			{
				switch (m_State)
				{
				case LexerState::before:
					m_String = &str;
					m_Current = m_String->begin();

					for (m_ActiveExpression = m_Expressions.begin(); m_ActiveExpression != m_Expressions.end(); ++m_ActiveExpression)
					{
 						if ((*m_ActiveExpression).second->Lex(m_State, *m_String, m_Current))
						{
							//m_ActiveExpression = { thing.first, thing.second };
							//Token<TokenType>(thing.first, std::string(thing.second->Start(), thing.second->End()));
							m_State = LexerState::inside;
							m_Type = (*m_ActiveExpression).first;
							break;
						}
					}

					//++m_Current;
					break;
				case LexerState::inside:
				case LexerState::after:
					if (m_Current != m_String->end())
					{
						auto & expr = *m_ActiveExpression;
						if (expr.second->Lex(m_State, *m_String, m_Current))
						{
							if (m_State == LexerState::after)
							{
								m_TokEnd = m_Current;

							}
							else
							{
								m_TokStart = m_Current;
								m_State = LexerState::after;
							}
						}
					}
					else
					{
						m_State = LexerState::before;
					}
					break;
				default:
					throw(std::runtime_error("invalid lexer state"));
				}

				return *this;
				/*if (State == State.before) State = str;

				for (auto thing : m_Expressions)
				{
					if (thing.second->Lex(State))
						return Token<TokenType>(thing.first, std::string(thing.second->Start(), thing.second->End()));
				}

				throw(std::invalid_argument("no lexeme match found"));*\/
			}

			inline const LexerState & GetState() const		{ return m_State; }
			inline Token GetToken() const					{ return{ (*m_ActiveExpression).first, m_TokStart, m_TokEnd } }

			inline Lexer & operator()(const std::string & str)			{ return Lex(str); }
		};*/
	}

	enum eLexToken
	{
		LEX_EOF = -1,
		LEX_EOL = LEX_EOF,		// no more
		LEX_NULL = 0,			// nada
		LEX_PUNCTUATOR,			// stuff that separates the other stuff, but isn't a whitespace
		LEX_NUMBER,				// duhhh
		LEX_LABEL,				// identifiers before :
		LEX_STRING,				// "all ovv thisssss"
		LEX_OPERATOR,			// subset of punctuators
		LEX_IDENTIFIER,			// the nouns of programming languages
		LEX_DIRECTIVE,			// #anythinglikethis
		LEX_SUBSCRIPT,			// not_this[but_this]
		LEX_UNKNOWN,				// wtf?
	};

	class CSCRLexer
	{
		friend class CTokenInfo;
		static inline bool			IsIdentifier(int ch)
		{
			return IsAlphanumeric(ch) || ch == '_' || ch == '.';		// R* likes .'s too
		}
		static inline bool			IsNumber(int ch)
		{
			return IsDecimal(ch) != 0;
		}
		static inline bool			IsPunctuator(int ch)
		{
			switch (ch)
			{
			case ',': case ':': case '&': case '\\':
			case '{': case '}':
			case '(': case ')':
			case '[': case ']':
				return true;
			}
			return IsOperator(ch);
		}
		static inline bool			IsOperator(int ch)
		{
			switch (ch)
			{
			case '<': case '>':
			case '#': case '=':
			case '+': case '-': case '*': case '/':
			case '@':
				// (new)
			case '^': case '~': case '%':
				return true;
			}
			return false;
		}
		static inline bool			SkipWhiteSpace(const char *&str)
		{
			auto old = str;
			while (IsSpace(*str)) ++str;
			return old != str;
		}

	public:
	};

	class CTokenInfo
	{
		eLexToken					m_eType;
		const char				*	m_szTokenBegin;
		const char				*	m_szTokenEnd;

		inline void			Nullify()
		{
			m_eType = LEX_NULL;
			m_szTokenBegin = nullptr;
			m_szTokenEnd = nullptr;
		}

	public:
		CTokenInfo()
		{
			Nullify();
		}
		CTokenInfo(const char * szCode)
		{
			Generate(szCode);
		}
		inline eLexToken		GetType() const							{ return m_eType; }
		inline const char *	GetBegin() const							{ return m_szTokenBegin; }
		inline const char *	GetEnd()	 const							{ return m_szTokenEnd; }
		inline std::string	GetString()	const						{ return std::string(m_szTokenBegin, m_szTokenEnd); }
		inline size_t		GetLength() const						{ return m_szTokenEnd - m_szTokenBegin; }

		bool					Generate(const char * szCode)
		{
			m_eType = LEX_NULL;

			while (*szCode)
			{
				// skip spaces
				if (CSCRLexer::SkipWhiteSpace(szCode))
					continue;
				// check for directive
				if (*szCode == '#')
				{
					CSCRLexer::SkipWhiteSpace(++szCode);
					m_szTokenEnd = m_szTokenBegin = szCode;
					do ++m_szTokenEnd;
					while (*m_szTokenEnd && !IsSpace(*m_szTokenEnd) && !CSCRLexer::IsPunctuator(*m_szTokenEnd));
					m_eType = LEX_DIRECTIVE;
				}
				// check for string
				else if (*szCode == '"')
				{
					m_szTokenEnd = m_szTokenBegin = ++szCode;
					bool bEscape = false;
					while (*m_szTokenEnd)
					{
						if (*m_szTokenEnd == '"')
						{
							if (!bEscape) break;
							else bEscape = false;
						}
						else if (*m_szTokenEnd == '\\')
							bEscape = bEscape != true;
						else bEscape = false;
						++m_szTokenEnd;
					}
					m_eType = LEX_STRING;
				}
				// check for subscripts
				else if (*szCode == '[')
				{
					int nDepth = 1;
					m_szTokenEnd = m_szTokenBegin = ++szCode;
					for (; *m_szTokenEnd; ++m_szTokenEnd)
					{
						if (*m_szTokenEnd == ']')
						{
							if (!--nDepth) break;
						}
						else if (*m_szTokenEnd == '[') ++nDepth;
					}
					m_eType = LEX_SUBSCRIPT;
				}
				// check for value
				else if (CSCRLexer::IsNumber(*szCode) || ((*szCode == '+' || *szCode == '-') && CSCRLexer::IsNumber(szCode[1])))
				{
					m_szTokenEnd = m_szTokenBegin = szCode;
					do ++m_szTokenEnd;
					while (*m_szTokenEnd && !IsSpace(*m_szTokenEnd) && !CSCRLexer::IsPunctuator(*m_szTokenEnd));
					m_eType = LEX_NUMBER;
				}
				// check for operator
				else if (CSCRLexer::IsOperator(*szCode))
				{
					m_szTokenEnd = m_szTokenBegin = szCode;
					do ++m_szTokenEnd;
					while (*m_szTokenEnd && CSCRLexer::IsOperator(*m_szTokenEnd));
					m_eType = LEX_OPERATOR;
				}
				// check for identifier / label
				else if (CSCRLexer::IsIdentifier(*szCode))
				{
					m_szTokenEnd = m_szTokenBegin = szCode;
					do ++m_szTokenEnd;
					while (*m_szTokenEnd && !IsSpace(*m_szTokenEnd) && !CSCRLexer::IsPunctuator(*m_szTokenEnd));
					m_eType = *m_szTokenEnd == ':' ? LEX_LABEL : LEX_IDENTIFIER;
				}
				// return any other punctuator
				else if (CSCRLexer::IsPunctuator(*szCode))
				{
					m_szTokenEnd = m_szTokenBegin = szCode;
					++m_szTokenEnd;
					m_eType = LEX_PUNCTUATOR;
				}
				else
				{
					m_szTokenEnd = m_szTokenBegin = szCode;
					do ++m_szTokenEnd;
					while (*m_szTokenEnd && !IsSpace(*m_szTokenEnd) && !CSCRLexer::IsIdentifier(*m_szTokenEnd) && !CSCRLexer::IsPunctuator(*m_szTokenEnd));
					m_eType = LEX_UNKNOWN;
				}
				break;
			}
			if (m_eType != LEX_NULL) return true;
			m_szTokenBegin = nullptr;
			m_szTokenEnd = nullptr;
			return true;
		}
	};
}