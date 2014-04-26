#pragma once

#include "pugixml\pugixml.hpp"

namespace SCRambl
{
	class CXMLFile
	{
		pugi::xml_document		m_Document;
		pugi::xml_parse_result	m_Result;

	public:
		CXMLFile(const char * szFilePath) : m_Result(m_Document.load_file(szFilePath))
		{
		}

		operator bool() const
		{
			return m_Result == pugi::status_ok;
		}
	};
}