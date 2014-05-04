/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Containers.h"

namespace SCRambl
{
#define DIRECTORY_MANAGEMENT_INTERVAL 2000

	enum eFileAction { FILE_ACTION_NONE, FILE_ACTION_CREATE, FILE_ACTION_DELETE, FILE_ACTION_CHANGE, };
	enum eFileMode { FILE_MODE_READ = 1, FILE_MODE_WRITE = 2, FILE_MODE_READWRITE = FILE_MODE_READ|FILE_MODE_WRITE, };
	enum eFileType { FILE_TYPE_BINARY = 1, FILE_TYPE_TEXT = 2, FILE_TYPE_MAPPED = 4, };

	typedef uint32_t DIR_NOTIFICATION_CALLBACK(class CFileData*, eFileAction, void*);

	inline FILE		*	OpenExecutable(const char * szCmd, const char * szMode)
	{
		return _popen(szCmd, szMode);
	}
	inline FILE		*	OpenFile(const char *szFilePath, const char *szOpenMode)
	{
		return _wfopen(widen(szFilePath).c_str(), widen(szOpenMode).c_str());
	}
	inline int			CloseExecutable(FILE *hFile)
	{
		return _pclose(hFile);
	}
	inline void			CloseFile(FILE *hFile)
	{
		fclose(hFile);
	}
	inline char		*	GetWorkingDir(char * dest, size_t size)
	{
		std::wstring wstr;
		wstr.resize(size);
		_wgetcwd(&wstr[0], size);
		strncpy(dest, narrow(wstr).c_str(), size);
		dest[size - 1] = '\0';
		return dest;
	}
	inline char * GetFileDirectory(const char *szFilePath, char *dest, size_t size = 0)
	{
		const char * pStr1 = strrchr(szFilePath, '\\');
		if (!pStr1) *dest = '\0';
		else
		{
			if (!size || (size > (size_t)(pStr1 - szFilePath))) size = pStr1 - szFilePath + 1;
			szncpy(dest, szFilePath, size);
		}
		return dest;
	}
	inline bool FileCopy(const char *szSrcPath, const char * szDestPath, bool bOverwrite)
	{
		return CopyFileW(widen(szSrcPath).c_str(), widen(szDestPath).c_str(), bOverwrite != true) != false;
	}
	inline long FileGetCursorPos(FILE *hFile)
	{
		return ftell(hFile);
	}
	inline int SeekFile(FILE *hFile, long offset, int origin = SEEK_SET)
	{
		return fseek(hFile, offset, origin);
	}
	inline size_t FileGetSize(FILE *hFile)
	{
		DWORD p = FileGetCursorPos(hFile);
		SeekFile(hFile, 0, SEEK_END);
		DWORD e = FileGetCursorPos(hFile);
		SeekFile(hFile, p, SEEK_SET);
		return e;
	}
	inline int IsFileError(FILE *hFile)
	{
		return ferror(hFile);
	}
	inline bool IsEndOfFile(FILE *hFile)
	{
		return ferror(hFile) || feof(hFile);
	}
	inline uint64_t FiletimeToInt64(const FILETIME &ft)
	{
		return JoinInt32(ft.dwHighDateTime, ft.dwLowDateTime);
	}
	inline bool WriteStringToFile(FILE *hFile, const char *str)
	{
		return fputs(str, hFile) >= 0;
	}
	inline bool ReadStringFromFile(FILE *hFile, char *str, size_t len)
	{
		return fgets(str, len, hFile) == str;
	}
	inline size_t ReadFromFile(FILE *hFile, void *buff, size_t len)
	{
		return fread(buff, sizeof(char), len, hFile);
	}
	inline size_t WriteToFile(FILE *hFile, const void *buff, size_t len)
	{
		return fwrite(buff, sizeof(char), len, hFile);
	}

	class CFileData
	{
		friend class CDirectory;
		class CDirectory		*	m_pDirectory;
		char						m_szFilePath[MAX_PATH];
		const char			*	m_szFileExt;
		const char			*	m_szFileName;
		uint64_t					m_nLastModifiedTime;
		WIN32_FIND_DATAW			m_FindData;

	public:
		CFileData(const char * szFilePath, CDirectory * pDir, WIN32_FIND_DATAW * pFindData = nullptr);

		inline CDirectory				*	GetDirectory() const					{ return m_pDirectory; }
		inline const char				*	GetFilePath() const					{ return m_szFilePath; }
		inline const char				*	GetFileName() const					{ return m_szFileName; }
		inline const char				*	GetFileExt()	 const					{ return m_szFileExt; }
		inline const WIN32_FIND_DATAW	*	GetFindData() const					{ return &m_FindData; }
		inline bool							IsNamed(const char * szName) const	{ return *GetFileName() ? !strcmp(GetFileName(), szName) : false; }
		char * GetName(char * pOut, size_t len) const
		{
			size_t size = GetFileExt() - GetFileName();
			return szncpy(pOut, GetFileName(), len >= size ? size : len);
		}
		bool IsExtension(const char * szExt) const
		{
			return GetFileExt() ? !strcmp(szExt, GetFileExt()) : false;
		}
	};

	class CDirectory
	{
		friend class CDirectoryManager;

		HANDLE					m_hDirectoryChanges;
		std::string				m_sDirPath;
		const char			*	m_szDirName;
		uint_fast32_t			m_dwHash;
		WIN32_FIND_DATAW			m_FindData;
		std::list<CFileData*>	m_Files;

		DIR_NOTIFICATION_CALLBACK * m_pFunc;

	public:
		CDirectory(const char * szDirPath, const char * szFilePattern = "*");
		~CDirectory();

		inline const char * GetPath() const						{ return m_sDirPath.c_str(); }
		inline uint_fast32_t GetHash() const						{ return m_dwHash; }
		inline const std::list<CFileData*> & Files() const		{ return m_Files; }

		void FindFiles(const char * szFilePattern = "*");

		void SetHandler(DIR_NOTIFICATION_CALLBACK * pFunc)
		{
			m_pFunc = pFunc;
		}

		void UpdateNotifications();

		CFileData * GetFileDataWithName(const char * szName);
	};

	class CFile
	{
		void				*	m_pFile;
		CFileData		*	m_pData;
		CDirectory		*	m_pDirectory;
		long					m_nCurrentLine;
		size_t				m_nCurrentOffset;
		size_t				m_nSize;
		uint64_t				m_nLastModifiedTime;
		eFileMode			m_eFileMode;
		bool					m_bIsMapped;
		HANDLE				m_hMap;
		void				*	m_pvMapData;
		void				*	m_pvMapCur;
		char					m_szFilePath[MAX_PATH];
		char					m_szFileDir[MAX_PATH];
		const char		*	m_szFileName;

		void	 Init(const char * szFilePath, eFileMode mode, eFileType type);

		template<typename T>
		size_t WriteToMap(T val)
		{
			ASSERT(m_bIsMapped);
			*(T*)m_pvMapCur = val;
			m_pvMapCur += sizeof(T);
			return sizeof(T);
		}
		template<typename T>
		T ReadFromMap() const
		{
			ASSERT(m_bIsMapped);
			T * p = (T*)m_pvMapCur;
			m_pvMapCur += sizeof(T);
			return *p;
		}
		template<typename T>
		T * ReadFromMap() const
		{
			ASSERT(m_bIsMapped);
			T *p = (T*)m_pvMapCur;
			m_pvMapCur += sizeof(T);
			return p;
		}

	public:
		CFile(const char * szFilePath, eFileMode mode, eFileType type = FILE_TYPE_BINARY);
		CFile(CDirectory * pDir, const char * szFilePath, eFileMode mode, eFileType type = FILE_TYPE_BINARY);

		~CFile();

		bool			WriteLine(const char * szStr);
		bool			WriteLineFormat(const char * szFormat, ...);
		std::string	ReadLine() const;
		char		*	ReadLine(char * szLineOut, size_t size);
		size_t		Read(void *buff, size_t size);
		size_t		Write(const void *pBuff, size_t nLen);

		template<typename T>	size_t	Write(T val)	{ return m_bIsMapped ? WriteToMap<T>(val) : WriteToFile<T>(m_pFile, val); }
		template<typename T>	T		Read() const	{ return m_bIsMapped ? ReadFromMap<T>() : ReadFromFile<T>(m_pFile); }

		inline HANDLE	GetHandle() const		{
			ASSERT(m_bIsMapped);
			return m_pFile;
		}
		inline FILE	*	GetFilePtr() const		{
			ASSERT(!m_bIsMapped);
			return static_cast<FILE*>(m_pFile);
		}
		
		template<typename T>
		inline T		&	GetMapValue() {
			ASSERT(m_bIsMapped);
			auto p = (T*)m_pvMapCur;
			m_pvMapCur = (char*)m_pvMapCur + sizeof(T);
			return *p;
		}

		/*template<typename T>
		inline T		*	GetMapValue() const {
			ASSERT(m_bIsMapped);
			auto p = (T*)m_pvMapCur;
			m_pvMapCur += sizeof(T*);
			return p;
		}*/

		inline const char	*	GetName() const			{ return *m_szFileName ? m_szFileName : nullptr; }
		inline const char	*	GetPath() const			{ return *m_szFilePath ? m_szFilePath : nullptr; }
		inline int				GetLineNumber()	const	{ return m_nCurrentLine; }
		inline bool				HasEnded() const			{ return IsEndOfFile(GetFilePtr()); }
		inline size_t			GetPos()					{ return m_nCurrentOffset = FileGetCursorPos(GetFilePtr()); }
		inline size_t			GetPos() const			{ return FileGetCursorPos(GetFilePtr()); }
		inline size_t			GetSize() const			{ return m_nSize; }
		inline const __int64		GetModifiedTime() const	{
			return m_pData ? FiletimeToInt64(m_pData->GetFindData()->ftLastWriteTime) : m_nLastModifiedTime;
		}
		inline bool				OK() const				{ return m_pFile ? true : false; }
	};

	class CDirectoryManager
	{
		CTree<CDirectory*, DWORD>							m_DirectoryTree;
		CList<CDirectory*>								m_DirectoryList;
		DIR_NOTIFICATION_CALLBACK						m_pNotificationFunc;
		HANDLE											m_hManagementThread;

		static unsigned long WINAPI ManageDirectories(void * pData);

	public:
		CDirectoryManager();
		~CDirectoryManager();

		CDirectory * Add(const char * szDirPath, const char * szFilePattern = "*");
	};

	extern CDirectoryManager DirManager;

} // namespace SCRambl