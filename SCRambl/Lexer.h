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
	namespace Lexer
	{
		/*
		* LexerState stores global state information about the in-use Lexer
		*/
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

		/*
		* Scanner is a base interface for token scanners
		*/
		class Scanner
		{
		public:
			virtual bool Scan(const LexerState & state, const std::string & str, std::string::const_iterator & it)
			{
				if (it != str.end()) ++it;
				return false;
			}
		};

		/*
		* LexerToken holds a copy of token information, with the token ID type externally defined to represent the scanner which produced it
		*/
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

		/*
		* The Lexer is a scanning engine, bridging the gaps between token scanners and the parser
		*/
		template<typename TokenIDType>
		class Lexer
		{
			using ScannerList = std::vector<std::pair<TokenIDType, Scanner&>>;

			LexerState						m_State;
			std::string::iterator			m_Start;
			std::string::iterator			m_Current;
			const std::string			*	m_pString;
			
			ScannerList						m_Scanners;
			typename ScannerList::iterator	m_ScannerIter;
			Scanner						*	m_Scanner = nullptr;

		public:
			inline LexerState				&	GetState()				{ return m_State; }
			inline const LexerState			&	GetState() const		{ return m_State; }
			inline TokenIDType					GetTokenType() const	{ ASSERT(m_ScannerIter != m_Scanners.end());  return m_ScannerIter[0].first; }
			inline std::string::iterator		GetTokenStart()	const	{ ASSERT(m_Start != m_pString->end());  return m_Start; }
			inline std::string::iterator		GetTokenEnd() const		{ return m_Current; }
			inline LexerToken<TokenIDType>		Tokenize() const		{ return{ m_ScannerIter[0].first, m_Start, m_Current }; }

			virtual ~Lexer()
			{
			}

			/*
			* Adds a token scanner to this Lexer, causing Scan to attempt more detections on input
			* The token ID will be used by the lexer to represent which scanner detected a token
			* The passed scanner should override the 'Scan' method of 'Lexer::Scanner' with state-dependent lexeme rules
			*/
			template<class Scanner>
			void AddTokenScanner(TokenIDType tokenid, Scanner & scanner)
			{
				m_Scanners.emplace_back(tokenid, scanner);
			}

			/*
			* Use only when the Lexer is in the 'before' state
			* This function expects a source string and iterator ref with the position of the symbols to be lexed
			* The passed iterator may be changed by this function if the scanner found a prefix
			* Returns true if the state advances and a token scanner is found for the symbol
			*/
			bool Scan(const std::string & str, std::string::iterator & it)
			{
				//ASSERT(m_State == LexerState::before);

				// The scanner should return the number of characters that have been matched
				// The state will be advanced on the first nonzero value
				m_pString = &str;
				m_Start = it;
				m_Current = it;

				if (m_State == LexerState::before)
				{
					for (m_ScannerIter = m_Scanners.begin(); m_ScannerIter != m_Scanners.end(); ++m_ScannerIter)
					{
						auto & scanner = m_ScannerIter[0].second;

						if (scanner.Scan(m_State, str, m_Current))
						{
							m_Scanner = &scanner;
							m_State = LexerState::inside;
							it = m_Current;
							return true;
						}
						else m_Current = m_Start;
					}
				}
				return false;
			}

			/*
			* Use for inside/after states only
			* This function expects the iterator used in the initial Scan call, which holds the current lexing position
			* The passed iterator will represent the last scanned character on return
			* Returns true if the state advances (still false, however, if the state resets to use a different scanner)
			*/
			bool Scan(std::string::const_iterator & it)
			{
				auto & str = *m_pString;
				switch (m_State)
				{
				case LexerState::inside:
					if (m_Scanner->Scan(m_State, str, m_Current))
					{
						m_State = LexerState::after;
					}
					it = m_Current;
					return false;

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
	}
}