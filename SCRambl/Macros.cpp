#include "stdafx.h"
#include "Macros.h"

namespace SCRambl
{
	const MacroMap Macros::g_PredefinedMacros = {
		// Name					Macro()
		{ "__SCRAMBL",			Macro("__SCRAMBL") },
		{ "__SCRAMBL_VER",		Macro("__SCRAMBL_VER", MacroCode({ "0x01000101" })) },
		{ "__SCR",				Macro("__SCR", MacroCode({ "0x201300" })) }
	};

	///Macro::Macro()
	//{}

	Macro::Macro(const MacroName & name) : m_Name(name)
	{}

	Macro::Macro(const MacroName & name, const MacroCode & code) : m_Name(name), m_Code(code)
	{}

	Macro::~Macro()
	{}

	const MacroCode * Macros::Get(const MacroName & name) const
	{
		auto it = m_Map.find(name);
		return it != m_Map.end() ? &it->second.Code() : nullptr;
	}

	void Macros::Define(const MacroName & name, const MacroCode & code)
	{
		m_Map.insert(std::make_pair(name, Macro(name, code)));
	}

	void Macros::Undefine(const MacroName & name)
	{
		m_Map.erase(name);
	}

	size_t Macros::Size() const
	{
		return m_Map.size();
	}

	Macros::Macros() : m_Map(g_PredefinedMacros)
	{}
}