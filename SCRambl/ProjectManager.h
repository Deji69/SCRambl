/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
// Definitions for management of SCR projects
#pragma once

#include "Environment.h"
#include "utils\xml.h"
#include <list>

namespace SCRambl
{
	class ProjectFile
	{
		CXMLFile			m_XMLFile;

		ProjectFile(const char * szName, const char * szFilePath) : m_XMLFile(szFilePath)
		{
		}
	};

	/*class CProjectDir
	{

	};*/

	class Project
	{
		std::list<ProjectFile*>		m_Files;
	};

	Project					*	s_pProject;

} // namespace SCRambl