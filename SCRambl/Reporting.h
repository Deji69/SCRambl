/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <vector>
#include <string>
#include "utils.h"

namespace SCRambl
{
	namespace Reporting
	{
		/*// Report script processing stuff
		class Report
		{
		public:
			enum Type {
				notice, warning, error, fatal
			};

		private:
			Type				m_Type;
			std::string			m_Message;
			Script::Position	m_Where;

		protected:
			Report(Type type, std::string msg) : m_Type(type), m_Message(msg)
			{
			}

		public:
			inline Type		GetType() const			{ return m_Type; }
		};

		template<typename IDType>
		class Error : public Report
		{
			IDType			m_ID;

		public:
			Error(std::string msg) : Report(error, msg)
			{
			}
		};*/
	}
}