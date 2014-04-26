#include "stdafx.h"
#include "utils.h"
#include "FileSystem.h"

namespace SCRambl
{
	CDirectoryManager		DirManager;

	unsigned long CDirectoryManager::ManageDirectories(void * pData)
	{
		while (true)
		{
			for (auto pLink = DirManager.m_DirectoryList.Begin(); pLink != DirManager.m_DirectoryList.Bad(); ++pLink)
			{
				pLink->UpdateNotifications();
			}
			Sleep(DIRECTORY_MANAGEMENT_INTERVAL);
		}
	}

	CDirectory * CDirectoryManager::Add(const char * szDirPath, const char * szFilePattern)
	{
		static char szTemp[MAX_PATH];

		// Is this a full path? (check for drive letter or protocol)
		if (strchr(szDirPath, ':')) strcpy(szTemp, szDirPath);
		else
		{
			// If not, just append it to the working path to get the full path
			GetWorkingDir(szTemp, sizeof(szTemp));
			strcat(szTemp, "\\");
			strcat(szTemp, szDirPath);
		}

		// Oh no!
		size_t len = strlen(szTemp);

		// If \ is at the end, remove it
		while (szTemp[len-- - 1] == '\\');
		szTemp[len] = '\0';

		// No point creating CDirectory's to manage the same dir, check for a currently managed dir...
		DWORD dwHash = GenerateHash(szDirPath, len);
		if (auto pDir = m_DirectoryTree.Get(dwHash))
		{
			// Verify the find
			while (strcmp(pDir->GetPath(), szTemp))
			{
				pDir = m_DirectoryTree.Next();
				if (!pDir) break;
			}

			// Even if the filter is the same, we may still need to update the search...
			if (pDir) pDir->FindFiles(szFilePattern);
			return pDir;
		}

		// Create the dir, and add it to the containers
		auto pDir = new CDirectory(szTemp, szFilePattern);
		m_DirectoryList.Add(pDir);
		m_DirectoryTree.Add(dwHash, pDir);
		return pDir;
	}

	CFileData * CDirectory::GetFileDataWithName(const char * szName)
	{
		for (auto pLink = m_Files.begin(); pLink != m_Files.end(); ++pLink)
		{
			if ((*pLink)->IsNamed(szName))
				return *pLink;
		}
		return nullptr;
	}

	void CDirectory::FindFiles(const char * szFilePattern)
	{
		// Craft the pattern
		std::string pattern = m_sDirPath;
		pattern += "\\";
		pattern += szFilePattern;

		// Perform the file search
		HANDLE hFind = FindFirstFile(widen(pattern).c_str(), &m_FindData);
		if (hFind)
		{
			do
			{
				if (!(m_FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
				{
					if (!(m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						if (!(m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE))
						{
							std::string name = narrow(m_FindData.cFileName);
							if (!GetFileDataWithName(name.c_str()))
								m_Files.push_back(new CFileData(name.c_str(), this, &m_FindData));
						}
					}
				}
			} while (FindNextFile(hFind, &m_FindData));
			FindClose(hFind);
		}
	}

	void CDirectory::UpdateNotifications()
	{
		FILE_NOTIFY_INFORMATION fni[1024];
		DWORD dwBytes;
		memset(&fni, 0, sizeof(fni));

		FILE_NOTIFY_INFORMATION * pfni = fni;

		// Await directory changes
		for (DWORD dwOffset = 0; ReadDirectoryChangesW(m_hDirectoryChanges, &fni, sizeof(fni), false, FILE_NOTIFY_CHANGE_SECURITY
			| FILE_NOTIFY_CHANGE_CREATION
			| FILE_NOTIFY_CHANGE_LAST_ACCESS
			| FILE_NOTIFY_CHANGE_LAST_WRITE
			| FILE_NOTIFY_CHANGE_SIZE
			| FILE_NOTIFY_CHANGE_ATTRIBUTES
			| FILE_NOTIFY_CHANGE_DIR_NAME
			| FILE_NOTIFY_CHANGE_FILE_NAME,
			&dwBytes, NULL, NULL); dwOffset += dwBytes)
		{
			if (!*pfni->FileName) continue;

			std::string name = narrow(std::wstring(pfni->FileName, pfni->FileNameLength)).c_str();

			auto pFileData = GetFileDataWithName(name.c_str());
			if (!pFileData)
			{
				pFileData = new CFileData(name.c_str(), this);
				m_Files.push_back(pFileData);
			}
			switch (pfni->Action)
			{
			case FILE_ACTION_ADDED:
			case FILE_ACTION_MODIFIED:
			case FILE_ACTION_RENAMED_NEW_NAME:
				pFileData->m_nLastModifiedTime = GetTime();
				break;
			}
			pfni = (FILE_NOTIFY_INFORMATION*)((BYTE*)fni + dwOffset);
		}

		FindNextChangeNotification(m_hDirectoryChanges);
	}

	CDirectory::CDirectory(const char * szDirPath, const char * szFilePattern) :
		m_sDirPath(szDirPath)
	{
		m_szDirName = strrchr(m_sDirPath.c_str(), '\\') + 1;

		// Generate a hash for this directory so the manager doesn't create duplicates
		m_dwHash = GenerateHash(m_sDirPath.c_str());

		m_hDirectoryChanges = CreateFile(widen(szDirPath).c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

		if (szFilePattern) FindFiles(szFilePattern);
	}

	CDirectory::~CDirectory()
	{
		CloseHandle(m_hDirectoryChanges);
	}

	void CFile::Init(const char * szFilePath, eFileMode mode, eFileType type)
	{
		const char * szOpenMode;

		// Store paths and names
		const char * pStr = strrchr(szFilePath, '\\');
		GetWorkingDir(m_szFilePath, sizeof(m_szFilePath));
		strcat(m_szFilePath, "\\");
		strcat(m_szFilePath, szFilePath);
		GetFileDirectory(m_szFilePath, m_szFileDir);
		strcat(m_szFileDir, "\\");

		m_szFileName = m_szFilePath[strlen(m_szFileDir)] ? &m_szFilePath[strlen(m_szFileDir)] : nullptr;

		// Store stats
		m_nLastModifiedTime = GetFileModifiedTime(szFilePath);

		// If it's a mapped file, handle with care
		if (type == FILE_TYPE_MAPPED)
		{
			m_bIsMapped = true;

#ifdef WIN32
			m_pFile = CreateFile(widen(szFilePath).c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			bool b = m_pFile != INVALID_HANDLE_VALUE;
			ASSERT(b);
			if (b)
			{
				int rwflags;
				if (mode == FILE_MODE_READ) rwflags = PAGE_READONLY;
				else rwflags = PAGE_READWRITE;

				m_nSize = GetFileSize(GetHandle(), 0);
				m_hMap = CreateFileMapping(GetHandle(), 0, PAGE_READONLY, 0, 0, 0);
				ASSERT(m_hMap != NULL);
				
				if (m_hMap != NULL)
				{
					int flags;
					if (mode == FILE_MODE_READWRITE) flags = FILE_MAP_WRITE | FILE_MAP_READ;
					else if (mode == FILE_MODE_WRITE) flags = FILE_MAP_WRITE;
					else if (mode == FILE_MODE_READ) flags = FILE_MAP_READ;
					m_pvMapData = MapViewOfFile(m_hMap, flags, 0, 0, 0);
					ASSERT(m_pvMapData != NULL);

					if (m_pvMapData)
					{
						m_pvMapCur = m_pvMapData;
						return;
					}
					// TERMINATED
				}
				
				CloseHandle(m_pFile);
			}

			m_pFile = nullptr;
			m_nSize = -1;
#else
#error "wrong compiler"
#endif
			return;
		}
		else
		{
			// 
			if (mode == FILE_MODE_WRITE)
			{
				m_eFileMode = FILE_MODE_WRITE;
				szOpenMode = type == FILE_TYPE_BINARY ? "wb" : "wt";
			}
			else// if(mode == FILE_MODE_READ)
			{
				m_eFileMode = FILE_MODE_READ;
				szOpenMode = type == FILE_TYPE_BINARY ? "rb" : "rt";
			}

			if (m_pFile = OpenFile(szFilePath, szOpenMode))
			{
				m_nSize = FileGetSize(GetFilePtr());
				m_nCurrentOffset = 0;
				m_nCurrentLine = 0;
			}
			else
			{
				m_nSize = -1;
			}
			return;
		}
		// TERMINATED
	}

	CFile::CFile(const char * szFilePath, eFileMode mode, eFileType type) :
		m_pData(nullptr), m_bIsMapped(false)
	{
		Init(szFilePath, mode, type);
	}

	CFile::CFile(CDirectory * pDir, const char * szFilePath, eFileMode mode, eFileType type) :
		m_pDirectory(pDir), m_pData(nullptr), m_bIsMapped(false)
	{
		Init(szFilePath, mode, type);
		if (!OK()) return;

		auto & Files = m_pDirectory->Files();

		for (auto pLink = Files.begin(); pLink != Files.end(); ++pLink)
		{
			if (!strcmp(m_szFileName, (*pLink)->GetFileName()))
			{
				m_pData = *pLink;
				break;
			}
		}
	}

	bool	 CFile::WriteLine(const char * szStr)
	{
		if (m_eFileMode != FILE_MODE_WRITE) throw("Attempt to write to a read-only file (internal error)");
		return WriteStringToFile(GetFilePtr(), szStr);
	}

	bool CFile::WriteLineFormat(const char * szFormat, ...)
	{
		static char szTemp[8192];
		va_list args;
		va_start(args, szFormat);
		vsprintf(szTemp, szFormat, args);
		va_end(args);
		ASSERT((m_eFileMode & FILE_MODE_WRITE));
		return WriteStringToFile(GetFilePtr(), szTemp);
	}

	std::string CFile::ReadLine() const
	{
		ASSERT(m_bIsMapped);
		return "";
	}

	char	 * CFile::ReadLine(char * szLineOut, size_t nSize)
	{
		ASSERT((m_eFileMode & FILE_MODE_READ));
		if (ReadStringFromFile(GetFilePtr(), szLineOut, nSize))
		{
			++m_nCurrentLine;
			return szLineOut;
		}
		return nullptr;
	}

	size_t CFile::Read(void *buff, size_t size)
	{
		ASSERT((m_eFileMode & FILE_MODE_READ));
		return ReadFromFile(GetFilePtr(), buff, size);
	}

	size_t CFile::Write(const void *buff, size_t size)
	{
		ASSERT((m_eFileMode & FILE_MODE_WRITE));
		return WriteToFile(GetFilePtr(), buff, size);
	}

	CFile::~CFile()
	{
		if (m_pFile)
		{
			if (m_bIsMapped)
			{
				if (m_pvMapData) UnmapViewOfFile(m_pvMapData);
				if (m_hMap) CloseHandle(m_hMap);
				CloseHandle(GetHandle());
			}
			else CloseFile(GetFilePtr());
		}
	}

	CDirectoryManager::CDirectoryManager()
	{
		m_hManagementThread = CreateThread(nullptr, 0, &ManageDirectories, nullptr, NULL, nullptr);
	}

	CDirectoryManager::~CDirectoryManager()
	{
		CloseHandle(m_hManagementThread);
	}

	CFileData::CFileData(const char * szFilePath, CDirectory * pDir, WIN32_FIND_DATAW * pFindData) : m_pDirectory(pDir),
		m_nLastModifiedTime(0)
	{
		if (pFindData) memcpy(&m_FindData, pFindData, sizeof(WIN32_FIND_DATAA));

		if (strchr(szFilePath, ':')) szncpy(m_szFilePath, szFilePath, sizeof(m_szFilePath));
		else
		{
			szncpy(m_szFilePath, pDir->GetPath(), sizeof(m_szFilePath));
			strcat(m_szFilePath, "\\");
			strcat(m_szFilePath, szFilePath);
		}

		if (m_szFileName = strrchr(m_szFilePath, '\\'))
			++m_szFileName;

		if (m_szFileExt = strrchr(m_szFileName, '.'))
			++m_szFileExt;
	}
} // namespace SCRambl