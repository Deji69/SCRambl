#pragma once

#include <stdint.h>
#include <string>
#include "hash.h"

namespace SCRambl
{
	class CKey
	{
		// stolen from CIdentifier
		std::string			m_sName;
		uint_fast32_t		m_dwHash;

	public:
		CKey(const char *szName) : m_sName(szName)
		{
			if (szName && *szName)
				m_dwHash = GenerateHash(m_sName.c_str());
			else
				m_dwHash = 0xFFFFFFFF;
		}
		~CKey()
		{
		}

	private:
		inline bool IsEqual(const CKey &v) const						{ return GetHash() == v.GetHash() && GetNameLength() == v.GetNameLength(); }

	public:
		inline size_t			GetNameLength()	 const				{ return m_sName.length(); }
		inline const char	*	GetName() const						{ return m_sName.c_str(); }
		inline void				GetName(char *szBuf) const			{ strcpy(szBuf, m_sName.c_str()); }
		inline uint_fast32_t		GetHash() const						{ return m_dwHash; }

		// slow success, slow fail (100% accurate)
		inline bool IsNamed(const char *szName) const				{ return strncmp(szName, m_sName.c_str(), m_sName.length()) == 0; }
		// slow success, fast fail (100% accurate)
		inline bool IsNamed(const char *szName, uint_fast32_t dwHash) const	{ return dwHash == GetHash() && IsNamed(szName); }
		// fast success, fast fail (~98% accurate)
		//inline bool IsNamed(DWORD dwHash) const					{return dwHash == GetHash();}
		// fast success, fast fail (~99.9% accurate)
		inline bool IsNamed(const CKey &v) const						{ return GetHash() == v.GetHash() && GetNameLength() == v.GetNameLength(); }
	};
}