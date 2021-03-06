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
	class Macro {
	public:
		class Name {
			friend class Macro;
			std::string m_Name;

			inline std::string& GetString() { return m_Name; }
			inline std::string& operator*() { return GetString(); }
			inline std::string* operator->() { return &**this; }

		public:
			Name() = default;
			Name(const std::string& str) : m_Name(str)
			{ }

			inline const std::string& GetString() const { return m_Name; }
			inline operator std::string() { return GetString(); }
			inline const std::string& operator*() const { return GetString(); }
			inline const std::string* operator->() const { return &**this; }
			inline operator const std::string&() const { return GetString(); }
		};
		class Code {
			friend class Macro;
			CodeLine m_Code;

			inline CodeLine& Symbols() { return m_Code; }
			inline CodeLine& operator*() { return Symbols(); }
			inline CodeLine* operator->() { return &**this; }

			inline void CopyCode(const CodeLine& code) {
				auto & symbols = Symbols();
				for (const auto& c : code) {
					// skip any whitespace at the beginning or consecutive
					if (c->GetType() == Symbol::whitespace) {
						if (symbols.Empty() || symbols.Back()->GetType() == Symbol::whitespace)
							continue;
					}

					if (c->GetType() != Symbol::eol) symbols.Append(c);
				}
			}

		public:
			Code() = default;
			Code(const CodeLine & code) {
				CopyCode(code);
			}

			inline const CodeLine & Symbols() const { return m_Code; }
			inline std::string String() const { return m_Code.String(); }

			inline operator CodeLine() { return Symbols(); }
			inline const CodeLine & operator*() const { return Symbols(); }
			inline const CodeLine * operator->() const { return &**this; }
			inline operator const CodeLine &() const { return Symbols(); }
		};

		Macro(const Name&);
		Macro(const Name&, const Code&);
		~Macro();

		inline const Name& GetName() const { return m_Name; }
		inline const Code& GetCode() const { return m_Code; }
		inline Code& GetCode() { return m_Code; }

	private:
		// stores the code within the macro, which can be more than one line
		Name m_Name;
		Code m_Code;
	};
	class MacroMap {
	public:
		using Map = std::map<std::string, Macro>;

		MacroMap() = default;
		MacroMap(const Map& predefined);

		const Macro* Get(const Macro::Name&) const;
		void Define(const Macro::Name&);
		void Define(const Macro::Name&, const Macro::Code&);
		void Undefine(const Macro::Name&);
		size_t Size() const;

	private:
		// macro macro maaaap... I wanna be, a macro map!
		Map m_Map;
	};
}