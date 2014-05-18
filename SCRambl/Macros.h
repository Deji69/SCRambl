/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once

#include <string>
#include <vector>
#include "Identifiers.h"

namespace SCRambl
{
	typedef IdentifierMap<class Macro> MacroMap;
	typedef std::string MacroCode;
	typedef Identifier MacroName;

	class Macro
	{
		// stores the code within the macro, which can be more than one line
		MacroName						m_Name;
		MacroCode						m_Code;

	public:
		//Macro();
		Macro(const MacroName &);
		Macro(const MacroName &, const MacroCode &);
		~Macro();

		inline const MacroName & Name() const			{ return m_Name; }
		inline const MacroCode & Code() const			{ return m_Code; }
		inline MacroCode & Code()						{ return m_Code; }
	};

	class CMacros
	{
		// macro macro maaaap... I wanna be, a macro map!
		static const MacroMap			g_PredefinedMacros;

		MacroMap							m_Map;

	public:
		CMacros();

		const MacroCode			*	Get(const MacroName &) const;
		void						Define(const MacroName &, const MacroCode &);
		//void						Define(const MacroName &, const std::string &);
		void						Undefine(const MacroName &);
		size_t						Size() const;
	};
}