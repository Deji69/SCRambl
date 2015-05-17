/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <unordered_map>
#include "Tasks.h"
#include "Configuration.h"
#include "Scripts.h"

namespace SCRambl
{
	class BuildConfig
	{

	};

	namespace BuildSystem
	{
		enum class BuildEvent
		{

		};
		enum class FileTypeType
		{

		};

		class FileType
		{
		public:
			enum Type {
				source, parsed, compiled, merged
			};

			FileType(Type type, std::string name, std::string ext);

			inline Type					GetType() const				{ return m_Type; }
			inline const std::string&	GetName() const				{ return m_Name; }
			inline const std::string&	GetExtension() const		{ return m_Extension; }

			static bool GetTypeByName(std::string name, Type& out) {
				static std::map<std::string, Type> s_map = {
					{ "source", source },
					{ "parsed", parsed },
					{ "compiled", compiled },
					{ "merged", merged }
				};
				auto it = s_map.find(name);
				if (it == s_map.end()) return false;
				out = it->second;
				return true;
			}

		private:
			Type						m_Type;
			std::string					m_Name;
			std::string					m_Extension;
		};

		class BuildDirectory
		{
			std::string					m_DirectoryPath;

		public:
			BuildDirectory() = default;
			BuildDirectory(std::string path) : m_DirectoryPath(path)
			{ }

			inline void SetDirectory(const std::string& path)				{ m_DirectoryPath; }
			inline const std::string& GetDirectory() const					{ return m_DirectoryPath; }

			inline operator const std::string&() const						{ return m_DirectoryPath; }
			inline operator std::string&()									{ return m_DirectoryPath; }
		};

		class BuildConfig
		{
			std::string	m_ID;
			std::string	m_Name;

			std::string m_DefinitionPath;
			std::vector<BuildDirectory> m_DefinitionPaths;
			std::set<std::string> m_UsedDefinitionPaths;
			std::string	m_LibraryPath;
			std::vector<std::string> m_IncludePaths;
			std::vector<std::string> m_LoadDefaults;

			std::unordered_map<std::string, std::shared_ptr<FileType>> m_FileTypes;

		public:
			using Shared = std::shared_ptr < BuildConfig > ;

			BuildConfig(std::string, std::string);
			BuildConfig(std::string, std::string, Configuration::Config&);

			std::shared_ptr<FileType> AddFileType(FileType::Type, std::string, std::string);
			std::shared_ptr<FileType> GetFileType(const std::string&);
			bool SetDefinitionPath(std::string);
			bool SetLibraryPath(const std::string&);
			bool AddDefinitionPath(const std::string&);
			bool AddIncludePath(const std::string&);
			bool AddDefaultLoad(const std::string &);
			bool AddDefaultLoadLocally(const std::string &);
			void OutputFile(const std::string&) const;

			const std::string & GetDefinitionPath() const;
			const std::string & GetLibraryPath() const;

			inline size_t GetNumDefinitionPaths() const						{ return m_DefinitionPaths.size(); }
			inline size_t GetNumDefaultLoads() const						{ return m_LoadDefaults.size(); }
			inline const std::string & GetDefinitionPath(size_t i) const	{ return m_DefinitionPaths[i]; }
			inline const std::string & GetDefaultLoad(size_t i) const		{ return m_LoadDefaults[i]; }
			inline const std::string & GetID() const						{ return m_ID; }
			inline const std::string & GetName() const						{ return m_Name; }
		};

		class Builder : public TaskSystem::Task < BuildEvent >
		{
			Engine& m_Engine;
			Configuration::Shared m_Config;
			Configuration::Config& m_ConfigurationConfig;
			std::unordered_map<std::string, std::shared_ptr<BuildConfig>> m_BuildConfigurations;
			std::shared_ptr<BuildConfig> m_BuildConfig;

		public:
			enum State {
				init, preprocess, parse, compile, link,
				finished
			};

			bool LoadScriptFile(const std::string &, Script &);
			bool SetConfig(const std::string &);
			std::shared_ptr<BuildConfig> GetConfig() const;

			Builder(Engine &);

		private:
			State		m_State;

			void Init();
			void Run();

		protected:
			void RunTask() override				{ Run(); }
			bool IsTaskFinished() override		{ return m_State == finished; }
			void ResetTask() override			{ Init(); }

		public:
		};

		class Build
		{
		};
	}
}