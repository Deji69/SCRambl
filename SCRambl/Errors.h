/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>

namespace SCRambl
{
	class Error
	{
		std::string			m_Message;

		virtual std::string BuildError(const std::string & msg)
		{
			return msg;
		}

	protected:
		Error() : m_Message(BuildError(""))
		{
		}
	public:
		explicit Error(const std::string & msg) : m_Message(BuildError(msg))
		{
		}
		const char * Message() const { return m_Message.c_str(); }
	};
}