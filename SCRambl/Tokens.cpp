#include "stdafx.h"
#include "Tokens.h"
#include "Scripts.h"

namespace SCRambl {
	namespace Tokens
	{
		/* Tokens::Map */
		Line* Map::AddLine(long long line) {
			auto it = m_Map.find(line);
			if (it != m_Map.end())
				return &it->second;
			auto pr = m_Map.emplace(line, Tokens::Line());
			if (pr.second) {
				return &pr.first->second;
			}
			return nullptr;
		}
		Tokens::Line* Tokens::Map::GetLine(long long line) {
			auto it = m_Map.find(line);
			return it != m_Map.end() ? &it->second : nullptr;
		}
	}
}