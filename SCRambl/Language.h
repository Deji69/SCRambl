/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Containers.h"
#include "utils.h"

namespace SCRambl
{
	class CLanguageEntry : public CKey
	{
		std::string			m_sText;

	public:
		CLanguageEntry(const char * szName, const char * str) : CKey(szName),
			m_sText(str)
		{
		}

		inline void				Set(const char * str)				{ m_sText = str; }
		inline const char	*	Get() const							{ return m_sText.c_str(); }
		inline size_t			Length() const						{ return m_sText.length(); }
	};

	class CLanguageTable : public CKey
	{
		friend class CLanguage;

		class CLanguage			*	m_pLanguage;
		size_t						m_nNumEntries;
		CTree<CLanguageEntry *, uint_fast32_t>		m_EntryTree;			// hehehe

		CLanguageTable(const char * szName, class CLanguage * pLang) : CKey(szName),
			m_pLanguage(pLang),
			m_nNumEntries(0)
		{
		}

	public:
		inline const CLanguageEntry	*	Get(const char * szEntry, uint_fast32_t dwHash) const
		{
			return m_EntryTree.Get(dwHash);
		}
		inline const CLanguageEntry	*	Get(const char * szEntry) const
		{
			return Get(szEntry, GenerateHash(szEntry));
		}
		inline void Add(CLanguageEntry * pEntry)
		{
			if (pEntry)
			{
				m_EntryTree.Add(pEntry->GetHash(), pEntry);
				++m_nNumEntries;
			}
		}
	};

	class CLanguage
	{
		CTree<CLanguageTable *, uint_fast32_t>		m_Tables;

	public:
		CLanguageTable		*		AddTable(const char * szName)
		{
			if (auto pLangTable = new CLanguageTable(szName, this))
			{
				m_Tables.Add(pLangTable->GetHash(), pLangTable);
				return pLangTable;
			}
			return nullptr;
		}
		const CLanguageTable		*	GetTable(const char * szName, uint_fast32_t dwHash) const
		{
			return m_Tables.Get(dwHash);
		}
		inline const CLanguageTable	*	GetTable(const char * szName) const
		{
			return GetTable(szName, GenerateHash(szName));
		}
	};
} // namespace SCRambl