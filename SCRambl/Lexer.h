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
#include "Symbols.h"

namespace SCRambl
{
	namespace Lexer
	{
		/*\
		 - Lexer::Scanner is a base interface for token scanners
		\*/
		class Scanner
		{
		public:
			virtual bool Scan(class State & state, CodeLine & line, CodeLine::iterator & sym) = 0;
			/*{
				if (*sym != Symbol::eol) ++sym;
				return false;
			}*/
		};

		/*\
		 - Lexer::Token holds a copy of token information, with the token ID type externally defined to represent the scanner which produced it
		\*/
		template<typename TokenIDType>
		class Token
		{
			TokenIDType						m_Type;
			std::string						m_Token;

		public:
			Token(TokenIDType type, std::string::const_iterator start, std::string::const_iterator end) : m_Type(type), m_Token(start, end)
			{}

			inline TokenIDType				GetType() const			{ return m_Type; }
			inline std::string				GetToken() const		{ return m_Token; }

			inline operator TokenIDType &()							{ return m_Type; }
			inline operator std::string &()							{ return m_Token; }
			inline operator TokenIDType () const					{ return m_Type; }
			inline operator const std::string &() const				{ return m_Token; }
		};

		/*\
		 - Lexer::State stores global state information about the in-use Lexer
		\*/
		class State
		{
			template<class>
			friend class Lexer;

		public:
			enum state_t { before, inside, after };

		private:
			// current state
			state_t							m_State = before;

			// current code symbol
			CodeLine::iterator				m_CodeCur;

			// *before* code symbol iterator
			CodeLine::iterator				m_CodeBefore;
			// *inside* code symbol iterator
			CodeLine::iterator				m_CodeInside;
			// *after* code symbol iterator
			CodeLine::iterator				m_CodeAfter;

			/*inline State & operator=(CodeLine::iterator code) {
				m_State = before;
				m_CodeBefore = m_CodeInside = m_CodeAfter = m_CodeCur = code;
				return *this;
			}*/
			explicit State(CodeLine::iterator code):
				m_CodeCur(code),
				m_CodeBefore(code),
				m_CodeInside(code),
				m_CodeAfter(code)
			{}

			inline CodeLine::iterator & Current()				{ return m_CodeCur; }
			inline CodeLine::iterator & Before()				{ return m_CodeBefore; }
			inline CodeLine::iterator & Inside()				{ return m_CodeInside; }
			inline CodeLine::iterator & After()					{ return m_CodeAfter; }

		public:
			State() = default;

			inline void ChangeState(state_t val)
			{
				switch (m_State = val)
				{
				case before:
					m_CodeBefore = m_CodeCur;
					break;
				case inside:
					m_CodeInside = m_CodeCur;
					break;
				case after:
					m_CodeAfter = m_CodeCur;
					break;
				default: BREAK();
				}

				//m_State = val;
			}

			//inline operator State &()			{ return m_State; }
			inline State & operator=(state_t op)		{ ChangeState(op); return *this; }
			inline bool operator!=(state_t op) const	{ return m_State == op; }
			inline operator state_t () const			{ return m_State; }
		};

		/*\
		 - The Lexer is a scanning engine, bridging the gaps between token scanners and the parser
		\*/
		template<typename TokenIDType>
		class Lexer
		{
			//using TokenScanner = std::pair<const TokenIDType, Scanner&>;
			//using ScannerList = std::vector<TokenScanner>;
			//using ScannerStates = std::vector<std::pair<State, TokenScanner&>>;
			typedef std::pair</*const */TokenIDType, Scanner&> TokenScanner;
			typedef std::vector<TokenScanner> ScannerList;
			typedef std::vector<std::pair<State, TokenScanner&>> ScannerStates;

			//State										m_State;
			enum Status { init, scanning, finished }	m_State = init;

			CodeLine::iterator					m_Start;
			CodeLine::iterator					m_Current;
			CodeLine						*	m_pCode = nullptr;
			
			ScannerList							m_Scanners;
			ScannerStates						m_ScannerStates;

			typename ScannerList::iterator		m_ScannerIter;
			Scanner							*	m_Scanner = nullptr;

		public:
			//inline State					&	GetState()				{ return m_State; }
			//inline const State				&	GetState() const		{ return m_State; }
			inline TokenIDType					GetTokenType()			{
				ASSERT(!m_ScannerStates.empty());
				const TokenScanner & r = m_ScannerStates.front().second;
				TokenIDType ty = r.first;
				return r.first;
			}
			inline CodeLine::iterator			GetTokenStart()			{ ASSERT(!m_ScannerStates.empty()); return m_ScannerStates.front().first.Before(); }
			inline CodeLine::iterator			GetTokenEnd()			{ ASSERT(!m_ScannerStates.empty()); return m_ScannerStates.front().first.After(); }
			inline Token<TokenIDType>			Tokenize() const		{ /*ASSERT(m_State == State::after);*/ ASSERT(!m_ScannerStates.empty()); return{ GetTokenStart(), GetTokenEnd(), m_Current }; }

			/*\
			 - Adds a token scanner to this Lexer, causing Scan to attempt more detections on input
			 - The token ID will be used by the lexer to represent which scanner detected a token
			 - Scanners override the 'Scan' method of 'Lexer::Scanner' with state-dependent lexeme rules
			\*/
			template<class Scanner> void AddTokenScanner(TokenIDType tokenid, Scanner & scanner)
			{
				m_Scanners.emplace_back(tokenid, scanner);
			}

			/*\
			 - This function expects a source CodeLine and iterator ref with the current symbol position
			 - Returns true if the state advances, or false on failure, causing a reset
			\*/
			bool Scan(CodeLine & str, CodeLine::iterator & it)
			{
				//ASSERT(m_State == State::before);

				// initialise the state with the starting symbol
				//m_State = it;

				// back up a pointer to the string
				if (m_pCode != &str)
				{
					m_pCode = &str;
					m_State = init;
				}

				//m_Start = it;
				//m_Current = it;

				switch (m_State)
				{
				case init:
					// throw a symbol at each scanner and keep any it sticks to...
					for (auto scit = m_Scanners.begin(); scit != m_Scanners.end(); ++scit)
					{
						// initialise the state
						State state(it);
						
						// scan beginning token symbol with the scanner
						auto & scanner = scit[0].second;
						if (scanner.Scan(state, str, state.Current()))
						{
							// match? store its state...
							m_ScannerStates.emplace_back(state, *scit);
						}
					}

					// if we found a scanner, notify the callee and start scanning
					if (!m_ScannerStates.empty())
					{
						m_State = scanning;
						return true;
					}
					break;

				case scanning:
					// 
					for (auto scit = m_ScannerStates.begin(); scit != m_ScannerStates.end(); ++scit)
					{
						// restore the scanners state
						State & state = scit[0].first;
						auto & scanner = scit[0].second.second;

						// scan inside the token
						if (!scanner.Scan(state, str, state.Current()))
						{
							// failed? do away with this scanner, then...
							//auto next = std::next(scit);
							scit = m_ScannerStates.erase(scit);
							//scit = std::prev(next);
							continue;
						}
					}
					return true;

				case finished:
					return true;
				}
				return false;
			}

			/*\
			 - Use for inside/after states only
			 - This function expects the iterator used in the initial Scan call, which holds the current lexing position
			 - The passed iterator will represent the last scanned character on return
			 - Returns true if the state advances (still false, however, if the state resets to use a different scanner)
			\*/
			/*bool Scan(std::string::const_iterator & it)
			{
				auto & str = *m_pCode;
				switch (m_State)
				{
				case State::inside:
					if (m_Scanner->Scan(m_State, str, m_Current))
					{
						m_State = State::after;
					}
					it = m_Current;
					return false;

				case State::after:
					if (m_Scanner->Scan(m_State, str, m_Current))
					{
						m_State = State::before;
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
								m_State = State::inside;
								break;
							}
							else m_Current = m_Start;
						}
						m_Current = m_Start;
						it = m_Current;
						m_State = State::before;
						return false;
					}
					break;
				default: throw(std::runtime_error("Invalid Lexer State"));
				}

				it = m_Current;
				return true;
			}*/
		};
	}
}