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
#include "Tokens.h"
#include "Scripts.h"

namespace SCRambl
{
	namespace Lexer
	{
		enum Result {
			found_nothing,
			found_token,
			still_scanning
		};

		/*\
		 - Lexer::Scanner is a base interface for token scanners
		\*/
		class Scanner
		{
		public:
			virtual bool Scan(class State & state, Script::Position & pos) = 0;
		};

		/*\
		 - Lexer::Token holds a copy of token information, with the token ID type externally defined to represent the scanner which produced it
		\*/
		template<typename TokenIDType>
		class Token
		{
			template<class>
			friend class Lexer;

			TokenIDType						m_Type;
			Script::Position				m_Start;
			Script::Position				m_Middle;
			Script::Position				m_End;

		public:
			Token()
			{ }

			inline TokenIDType				GetType() const			{ return m_Type; }

			// get the position of the token before any prefix
			inline Script::Position			Begin() const			{ return m_Start; }
			// get the position of the token after any prefix
			inline Script::Position			Inside() const			{ return m_Middle; }
			// get the position of the token at the very end
			inline Script::Position			End() const				{ return m_End; }
			// get the entire range
			inline Script::Range			Range() const			{ return{ m_Start, m_End }; }

			inline operator TokenIDType &()							{ return m_Type; }
			inline operator TokenIDType () const					{ return m_Type; }

			inline void operator ()(TokenIDType type, Script::Position start, Script::Position mid, Script::Position end) {
				m_Type = type;
				m_Start = start;
				m_Middle = mid;
				m_End = end;
			}
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

			// current code symbol position
			Script::Position				m_CodeCur;

			// *before* code symbol position
			Script::Position				m_CodeBefore;

			// *inside* code symbol position
			Script::Position				m_CodeInside;

			// *after* code symbol position
			Script::Position				m_CodeAfter;

			explicit State(Script::Position pos) :
				m_CodeCur(pos),
				m_CodeBefore(pos),
				m_CodeInside(pos),
				m_CodeAfter(pos)
			{}

			inline Script::Position & Current()				{ return m_CodeCur; }
			inline Script::Position & Before()				{ return m_CodeBefore; }
			inline Script::Position & Inside()				{ return m_CodeInside; }
			inline Script::Position & After()				{ return m_CodeAfter; }

		public:
			inline void ChangeState(state_t val)
			{
				switch (val)
				{
				case before:
					m_CodeBefore = m_CodeCur;
					break;
				case inside:
					m_CodeInside = m_CodeCur;
					break;
				case after:
					if (m_State == before) m_CodeInside = m_CodeBefore;
					m_CodeAfter = m_CodeCur;
					break;
				default: BREAK();
				}
				
				m_State = val;
			}

			//inline operator State &()			{ return m_State; }
			inline State & operator=(state_t op)		{ ChangeState(op); return *this; }
			inline bool operator==(state_t op) const	{ return m_State == op; }
			inline bool operator!=(state_t op) const	{ return !(*this == op); }
			inline operator state_t () const			{ return m_State; }
		};

		/*\
		 - The Lexer is a scanning engine, bridging the gaps between token scanners and the parser
		 - This wonderful class makes adding new code elements a 2-step process
		\*/
		template<typename TokenIDType>
		class Lexer
		{
		private:
			typedef std::pair<TokenIDType, Scanner&> TokenScanner;
			typedef std::vector<TokenScanner> ScannerList;
			typedef std::list<std::pair<State, TokenScanner&>> ScannerStates;
			
			ScannerList							m_Scanners;
			ScannerStates						m_ScannerStates;

		public:
			/*\
			 - Adds a token scanner to this Lexer, causing Scan to attempt more detections on input
			 - The token ID will be used by the lexer to represent which scanner detected a token
			 - Scanners override the 'Scan' method of 'Lexer::Scanner' with state-dependent lexeme rules
			 - They must change state manually, and return true while valid
			 - Usually each scanner only requires one call per the 3 states (before/inside/after)
			\*/
			template<class Scanner> void AddTokenScanner(TokenIDType tokenid, Scanner & scanner)
			{
				m_Scanners.emplace_back(tokenid, scanner);
			}

			/*\
			 - Give it the current Script::Position and a Token<> to write to, let magic happen
			\*/
			Result Scan(const Script::Position & pos, Token<TokenIDType> & token)
			{
				ASSERT(!m_Scanners.empty());		// you need to add some scanners first!

				// if we haven't any active scanners, start trying to activate them
				if (m_ScannerStates.empty())
				{
					// throw a symbol at each scanner and keep any it sticks to...
					for (auto scit = m_Scanners.begin(); scit != m_Scanners.end(); ++scit)
					{
						// initialise the state
						State state(pos);

						// scan beginning token symbol with the scanner
						auto & scanner = scit->second;
						if (scanner.Scan(state, state.Current()))
						{
							// match? store its state...
							m_ScannerStates.emplace_back(state, *scit);
						}
					}

					// no scanner found? our master will not be pleased...
					return m_ScannerStates.empty() ? found_nothing : still_scanning;
				}
				else
				{
					bool first = true;
					long longest = 0;

					// go through those scanners we found
					for (auto scit = m_ScannerStates.begin(); scit != m_ScannerStates.end(); first = false)
					{
						// restore the scanners state
						State & state = scit->first;
						auto & scanner = scit->second.second;

						// all scanners finishing first qualify for the major league
						bool last = state == State::after;

						// skip inactive scanners
						if (state != State::before)
						{
							// if a scanner throws an error, remove it from the active scanner list and pass the error along
							try{
								if (!scanner.Scan(state, state.Current()))
								{
									// failed? do away with this scanner, then...
									scit = m_ScannerStates.erase(scit);

									// if we've got more scanners, let them continue - otherwise give up
									if (!m_ScannerStates.empty()) continue;
									return found_nothing;
								}
							}
							catch (...) {
								// remove it
								scit = m_ScannerStates.erase(scit);
								// pass the error 
								throw;
							}
						}
						
						// as soon as the quickest scanner finishes, call the race
						if (last)
						{
							token(scit->second.first, state.Before(), state.Inside(), state.After());

							// if it's the only one, we're a success
							if (m_ScannerStates.size() <= 1 || scit == m_ScannerStates.begin())
							{
								m_ScannerStates.clear();
								return found_token;
							}
							else
							{
								// 
								scit = m_ScannerStates.erase(scit);
								continue;
							}
						}

						++scit;
					}
					return still_scanning;
				}
			}

			/*\
			 - ScanFor something specific - will initiate similarly to 'Scan' except for a specific token type
			 - After the first successful call, further calls made to this simply redirect to 'Scan'
			\*/
			Result ScanFor(TokenIDType type, const Script::Position & pos, Token<TokenIDType> & token)
			{
				ASSERT(!m_Scanners.empty());			// better add a scanner

				// if we haven't any active scanners, start trying to activate them
				if (m_ScannerStates.empty())
				{
					// the callee is picky, so only use the the scanner of the token type they specified
					for (auto scit = m_Scanners.begin(); scit != m_Scanners.end(); ++scit)
					{
						// no? continue;
						if (scit->first != type) continue;

						// initialise the state
						State state(pos);

						// scan beginning token symbol
						if (scit->second.Scan(state, state.Current()))
						{
							// matched! aaand we're done...
							m_ScannerStates.emplace_back(state, *scit);
							return still_scanning;
						}
					}
				}
				else return Scan(pos, token);

				return found_nothing;
			}
		};
	}
}