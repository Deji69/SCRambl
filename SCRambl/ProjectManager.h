/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
// Definitions for management of SCR projects
#pragma once
#include <list>
#include "utils\xml.h"
#include "Environment.h"
#include "Builder.h"

namespace SCRambl
{
	class ProjectFile
	{
		std::string					m_Path;

	public:
		ProjectFile(std::string path) : m_Path(path)
		{ }
	};

	class ProjectTarget
	{

	};

	class Project
	{
		std::string m_Name;
		BuildConfig::Shared m_Config;
		std::vector<ProjectFile> m_Sources;
		std::unordered_map<std::string, ProjectTarget> m_Targets;

	public:
		Project() = default;
		Project(std::string name) : m_Name(name)
		{ }

		inline void SetName(std::string name) { m_Name = name; }
		inline void SetConfig(BuildConfig::Shared conf) { m_Config = conf; }
		inline const std::string& GetName() const { return m_Name; }
		inline BuildConfig::Shared GetConfig() const { return m_Config; }

		inline void AddSource(std::string path) {
			m_Sources.emplace_back(path);
		}

		void LoadFile(const char*);
		void SaveFile(const char* = "") const;
	};

} // namespace SCRambl