/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>

namespace SCRambl
{
	// Integer constant type
	class IntConst
	{
		long long m_nVal;

		// amount of parsed characters in integer constant (index of first invalid character)
		size_t			m_nSize;

	public:
		IntConst(const std::string &);

		template<class T>
		inline T		Value() const				{ return m_nVal; }
		inline size_t	Pos() const					{ return m_nSize; }
		inline bool		IsSigned() const			{ return m_nVal < 0; }
	};

	// Floating-point constant type
	class FloatConst
	{
		double m_fVal;

		// amount of parsed characters in floating-point constant (index of first invalid character)
		size_t			m_nSize;

	public:
		FloatConst(const std::string &);

		template<class T>
		inline T		Value() const				{ return m_fVal; }
		inline size_t	Pos() const					{ return m_nSize; }
	};

	// Character literal
	class CharLiteral
	{
		int		m_nVal;
		size_t	m_nSize;

	public:
		CharLiteral(const std::string &);

		inline int	Value() const					{ return m_nVal; }
		inline size_t Pos() const					{ return m_nSize; }
	};

	// String literal
}