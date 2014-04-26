#pragma once

#include <vector>
#include <string>

namespace SCRambl
{
	class Error
	{
		unsigned long				m_nCode;
		std::vector<std::string>		m_Params;

	public:
		Error(unsigned long code, std::vector<std::string> params);
	};
}