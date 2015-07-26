#include "stdafx.h"
#include "Tokens.h"
#include "Scripts.h"

using namespace SCRambl;
using namespace SCRambl::Tokens;

// Tokens::Line
Line::Ref Line::GetToken(size_t col) {
	return Line::Ref(m_Line, col);
}
void Line::AddToken(Token* tok) {
	auto dest_it = m_Line.end();
	auto dest_col = -1;
	for (auto it = m_Line.begin(); it != m_Line.end(); ++it) {
		auto ptr = *it;
		auto col = ptr->GetPosition().GetColumn();

		if (dest_it == m_Line.end() || col < dest_col) {
			dest_it = it;
			dest_col = col;
		}
		else if (col >= dest_col) {
			if (col == dest_col) dest_col = -1;
			break;
		}
	}

	if (dest_it != m_Line.end()) {
		if (dest_col == -1) return;
		m_Line.emplace(dest_it, tok);
	}
	else m_Line.emplace_back(tok);
}
// Tokens::Map
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
Line* Map::GetLine(long long line) {
	auto it = m_Map.find(line);
	return it != m_Map.end() ? &it->second : nullptr;
}