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
#include "Symbols.h"

namespace SCRambl
{
	class Macro
	{
	public:
		class Name
		{
			friend class Macro;
			std::string				m_Name;

			inline std::string & GetString()	{ return m_Name; }

			inline std::string & operator*()	{ return GetString(); }
			inline std::string * operator->()	{ return &**this; }

		public:
			Name() = default;
			Name(const std::string & str) : m_Name(str)
			{}

			inline const std::string & GetString() const	{ return m_Name; }

			inline operator std::string()					{ return GetString(); }
			inline const std::string & operator*() const	{ return GetString(); }
			inline const std::string * operator->() const	{ return &**this; }
			inline operator const std::string &() const		{ return GetString(); }
		};

		class Code
		{
			friend class Macro;
			CodeLine				m_Code;

			inline CodeLine::vector & Symbols()				{ return m_Code.Symbols(); }

			inline CodeLine::vector & operator*()			{ return Symbols(); }
			inline CodeLine::vector * operator->()			{ return &**this; }

			inline void CopyCode(const CodeLine::vector & code)
			{
				auto & symbols = m_Code.Symbols();
				for (const auto & c : code)
				{
					// skip any whitespace at the beginning or consecutive
					if (c.GetType() == Symbol::whitespace)
					{
						if (symbols.empty() || symbols.back().GetType() == Symbol::whitespace)
							continue;
					}

					if (c.GetType() != Symbol::eol) symbols.push_back(c);
				}
			}

		public:
			Code() = default;
			Code(const CodeLine::vector & code)
			{
				CopyCode(code);
			}

			inline const CodeLine::vector & Symbols() const		{ return m_Code.Symbols(); }
			inline std::string String() const					{ return m_Code.String(); }

			inline operator CodeLine::vector()					{ return Symbols(); }
			inline const CodeLine::vector & operator*() const	{ return Symbols(); }
			inline const CodeLine::vector * operator->() const	{ return &**this; }
			inline operator const CodeLine::vector &() const	{ return Symbols(); }
		};

	private:
		// stores the code within the macro, which can be more than one line
		Name						m_Name;
		Code						m_Code;

	public:
		//Macro();
		Macro(const Name &);
		Macro(const Name &, const Code &);
		~Macro();

		inline const Name & GetName() const			{ return m_Name; }
		inline const Code & GetCode() const			{ return m_Code; }
		inline Code & GetCode()						{ return m_Code; }
	};

	class MacroMap
	{
		typedef std::map<std::string, class Macro> MacMap;

		// macro macro maaaap... I wanna be, a macro map!
		MacMap						m_Map;

	public:
		MacroMap() = default;
		MacroMap(const MacMap & predefined);

		const Macro::Code		*	Get(const Macro::Name &) const;
		void						Define(const Macro::Name &);
		void						Define(const Macro::Name &, const Macro::Code &);
		void						Undefine(const Macro::Name &);
		size_t						Size() const;
	};
}