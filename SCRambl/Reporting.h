#pragma once

namespace SCRambl
{
	class Error
	{
		unsigned long			m_nCode;
		unsigned long			m_nLine;

	public:
		Error(unsigned long nErrorCode, unsigned long nLine);
	};
}