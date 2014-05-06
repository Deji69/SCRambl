/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once

#include "Literals.h"

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
	// operators for preprocessor logic
	enum eExprLog {
		EXPR_LOG_NOT, EXPR_LOG_AND, EXPR_LOG_OR
	};

	static bool EvalLogic(bool bCurrent, bool bNotOp, bool bLogOr, bool bNew)
	{
		// oh the binary goodness
		return bNew ? (bLogOr ? bCurrent | !bNotOp : bCurrent & !bNotOp) : (bLogOr ? bCurrent | bNotOp : bCurrent & bNotOp);
	}

template<class SignedType, class UnsignedType>
	class IntExpression
	{
		eExprOp	m_Operation = EXPR_OP_INVALID,
				m_Unary = EXPR_OP_INVALID;
		eExprLog m_Logical = EXPR_LOG_NOT;
		bool m_bGotLOp = false, m_bGotROp = false;
		bool m_bUnsigned = false;
		bool m_bLogUnaryNot = false;

		union
		{
			SignedType m_nVal = 0;
			UnsignedType m_unVal;
		};

		SignedType Calculate(SignedType val)
		{
			switch (m_Operation)
			{
				// Unary
			case EXPR_OP_INC:		return ++m_nVal;
			case EXPR_OP_DEC:		return --m_nVal;
			case EXPR_OP_POS:		return +m_nVal;
			case EXPR_OP_NEG:		return -m_nVal;
			case EXPR_OP_BIT_NOT:	return ~m_nVal;

				// Arithmetic
			case EXPR_OP_ASSIGN:	return val;
			case EXPR_OP_ADD:		return m_nVal + val;
			case EXPR_OP_SUBTRACT:	return m_nVal - val;
			case EXPR_OP_MULTIPLY:	return m_nVal * val;
			case EXPR_OP_DIVIDE:	return m_nVal / val;

				// Bitwise
			case EXPR_OP_BIT_AND:	return m_nVal & val;
			case EXPR_OP_BIT_OR:	return m_nVal | val;
			case EXPR_OP_BIT_XOR:	return m_nVal ^ val;
			case EXPR_OP_BIT_LSH:	return m_nVal << val;
			case EXPR_OP_BIT_RSH:	return m_nVal >> val;
			case EXPR_OP_BIT_MOD:	return m_nVal % val;

				// Comparison
			/*case EXPR_OP_EQUAL:		return EvalLogic(m_nVal ? true : false, bLogicalNot, bLogicalOp, m_nVal == val);
			case EXPR_OP_INEQUAL:	return EvalLogic(m_nVal ? true : false, bLogicalNot, bLogicalOp, m_nVal != val);
			case EXPR_OP_EQGREATER:	return EvalLogic(m_nVal ? true : false, bLogicalNot, bLogicalOp, m_nVal >= val);
			case EXPR_OP_EQLESSER:	return EvalLogic(m_nVal ? true : false, bLogicalNot, bLogicalOp, m_nVal <= val);
			case EXPR_OP_GREATER:	return EvalLogic(m_nVal ? true : false, bLogicalNot, bLogicalOp, m_nVal > val);
			case EXPR_OP_LESSER:	return EvalLogic(m_nVal ? true : false, bLogicalNot, bLogicalOp, m_nVal < val);*/
			default:
				throw(std::invalid_argument("invalid integer expression operation"));
			}
		}

	public:
		//Expression(const Expression&) = delete;
		/*Expression() : m_Operation(EXPR_OP_INVALID),
			m_PostUnary(EXPR_OP_INVALID), m_PreUnary(EXPR_OP_INVALID),
			m_bGotLOp(false), m_bGotROp(false)
		{
		}*/

		template<class T>
		const T & Result() const
		{
			const T & a = m_bUnsigned ? m_unVal : m_nVal;
			return a;
		}

		void Evaluate(SignedType val)
		{
			if (m_bGotLOp)
			{
				Calculate(val);
			}
			else
			{
				m_nVal = val;
				m_bGotLOp = true;
			}
		}

		void Evaluate(UnsignedType val)
		{
			m_bUnsigned = true;

			if (m_bGotLOp)
			{
				Calculate(val);
			}
			else
			{
				m_unVal = val;
				m_bGotLOp = true;
			}
		}

		void Logicate(eExprLog op)
		{
			if (op == EXPR_LOG_NOT)
				m_bLogUnaryNot = !m_bLogUnaryNot;
			else if (m_Logical == EXPR_LOG_NOT)
				m_Logical = op;
		}

		void Operate(eExprOp op)
		{
			switch (op)
			{
			case EXPR_OP_POS:
			case EXPR_OP_NEG:
			case EXPR_OP_BIT_NOT:
				m_Unary = op;
				break;
			case EXPR_OP_INC:
			case EXPR_OP_DEC:
				// ERROR...?
				break;
			default:
				m_Operation = op;
				break;
			}
		}
	};
}