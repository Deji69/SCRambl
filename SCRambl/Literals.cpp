#include "stdafx.h"
#include "Literals.h"
#include "Parser.h"

namespace SCRambl
{
	IntConst::IntConst(const std::string & expr)
	{
		try
		{
			m_nVal = ExprToInt<long long>(expr, &m_nSize);
		}
		catch (...)
		{
			throw;
		}
	}
	FloatConst::FloatConst(const std::string & expr)
	{
		try
		{
			m_fVal = ExprToFlt<double>(expr, &m_nSize);
		}
		catch (...)
		{
			throw;
		}
	}

	CharLiteral::CharLiteral(const std::string & expr)
	{
		try
		{
			m_nVal = ExprToChar(expr, &m_nSize);
		}
		catch (...)
		{
			throw;
		}
	}
}