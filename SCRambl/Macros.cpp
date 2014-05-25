#include "stdafx.h"
#include "Macros.h"
#include <algorithm>
#include <string>

namespace SCRambl
{
	/*const MacroMap MacroMap::g_PredefinedMacros = {
		// Name					Macro()
		{ "__SCRAMBL",			Macro("__SCRAMBL") },
		{ "__SCRAMBL_VER",		Macro("__SCRAMBL_VER", MacroCode({ "0x01000101" })) },
		{ "__SCR",				Macro("__SCR", MacroCode({ "0x201300" })) }
	};*/

	///Macro::Macro()
	//{}

	Macro::Macro(const MacroName & name) : m_Name(name)
	{}

	Macro::Macro(const MacroName & name, const MacroCode & code) : m_Name(name), m_Code(code)
	{
		m_Code.erase(std::remove(m_Code.begin(), m_Code.end(), '\n'), m_Code.end());
	}

	Macro::~Macro()
	{}

	MacroMap::MacroMap(const MacMap & predefined) : m_Map(predefined)
	{
	}

	const MacroCode * MacroMap::Get(const MacroName & name) const
	{
		auto it = m_Map.find(name);
		return it != m_Map.end() ? &it->second.Code() : nullptr;
	}

	void MacroMap::Define(const MacroName & name, const MacroCode & code)
	{
		//m_Map.insert(std::make_pair(name, Macro(name, code)));
		m_Map.emplace(name, Macro(name, code));
	}

	void MacroMap::Undefine(const MacroName & name)
	{
		m_Map.erase(name);
	}

	size_t MacroMap::Size() const
	{
		return m_Map.size();
	}
}