/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once

namespace SCRambl
{
	// operations for preprocessor expression evaluations
	enum eExprOp {
		EXPR_OP_INVALID,
		EXPR_OP_ASSIGN,
		EXPR_OP_ADD, EXPR_OP_SUBTRACT, EXPR_OP_MULTIPLY, EXPR_OP_DIVIDE,
		EXPR_OP_EQUAL, EXPR_OP_INEQUAL, EXPR_OP_GREATER, EXPR_OP_LESSER,
		EXPR_OP_EQGREATER, EXPR_OP_EQLESSER,
		EXPR_OP_INC, EXPR_OP_DEC, EXPR_OP_NEG, EXPR_OP_POS,
		EXPR_OP_BIT_AND, EXPR_OP_BIT_OR, EXPR_OP_BIT_XOR, EXPR_OP_BIT_NOT,
		EXPR_OP_BIT_LSH, EXPR_OP_BIT_RSH, EXPR_OP_BIT_MOD,
	};

	class Expression
	{
		eExprOp	m_Operation = EXPR_OP_INVALID,
		m_PostUnary = EXPR_OP_INVALID,
		m_PreUnary = EXPR_OP_INVALID;
		bool m_bGotLOp, m_bGotROp;

		union
		{
			long long m_nVal;
			unsigned long long m_unVal;
		};

	public:
		Expression() : m_Operation(EXPR_OP_INVALID),
			m_PostUnary(EXPR_OP_INVALID), m_PreUnary(EXPR_OP_INVALID),
			m_bGotLOp(false), m_bGotROp(false)
		{
		}

		void ProcessValue(long long)
		{

		}
	};
}