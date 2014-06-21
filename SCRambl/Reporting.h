/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <vector>
#include <string>
#include "Scripts.h"
#include "Tasks.h"

namespace SCRambl
{
	namespace Reporting
	{
		class Report
		{
		public:
			enum Type {
				notice, warning, error, fatal
			};

		private:
			Type			m_Type;

		protected:
			Report(Type type) : m_Type(type)
			{}

		public:
			inline Type		GetType() const			{ return m_Type; }
		};

		template<typename IDType>
		class Error : public Report
		{
			IDType			m_ID;

		public:
			Error() : Report(error)
			{ }
		};
	}
}