/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
// Definitions for management of SCR projects
#pragma once

#include "Environment.h"
#include "FileSystem.h"
#include "utils\xml.h"
#include <list>

namespace SCRambl
{
	class CProjectFile : public CKey
	{
		CXMLFile			m_XMLFile;


		CProjectFile(const char * szName, const char * szFilePath) : CKey(szName), m_XMLFile(szFilePath)
		{
		}
	};

	/*class CProjectDir
	{

	};*/

	class CProject
	{
		std::list<CProjectFile*>		m_Files;
	};

	CProject					*	s_pProject;

} // namespace SCRambl