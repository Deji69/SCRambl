/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <vector>
#include <list>
#include <map>

class CCLP
{
	std::vector<const char*> m_CommandLine;
	std::list<std::string> m_MainOpts;
	std::map<std::string, std::list<std::string>> m_Options;
	std::map<int, std::string>	m_ShortOptions;

public:
	void AddFlag(std::string cmd, int shortcmd)
	{
		m_ShortOptions[shortcmd] = cmd;
	}

	void Parse()
	{
		std::string cmd;

		for (auto arg : m_CommandLine)
		{
			if (*arg == '-' || *arg == '/')
			{
				if (*arg == '-') ++arg;
				switch (*arg)
				{
				case '/':
					++arg;
				case '-':
					// --fullflag
					++arg;
					cmd = arg;
					cmd.erase(cmd.find_first_of(":="));
					arg += cmd.length();
					++arg;
					break;
				default:
					// A char flag -f
					if (m_ShortOptions.find(*arg) != m_ShortOptions.end())
					{
						cmd = m_ShortOptions[*arg++];
						m_Options[cmd];
					}
					else
					{
						// No full equivalent? Just treat it as full...
						cmd = *arg;
						auto f = cmd.find_first_of(":=");
						if (f != cmd.npos) cmd.erase(f);
						arg += cmd.length();
						m_Options[cmd];
					}

					if (*arg == ':' || *arg == '=') ++arg;
					break;
				}
			}
			
			// Unless we're at the end of a flag-only arg...
			if (*arg)
			{
				if (cmd.empty())
				{
					// Must be the main parameters
					m_MainOpts.push_back(arg);
				}
				else
				{
					// Additional parameters for the last flag
					m_Options[cmd].push_back(arg);
				}
			}
		}
	}

	bool IsFlagSet(std::string flag) const
	{
		return m_Options.find(flag) != m_Options.end();
	}

	size_t GetNumOpts() const
	{
		return m_MainOpts.size();
	}

	const std::list<std::string> & GetOpts() const
	{
		return m_MainOpts;
	}

	const std::list<std::string> & GetFlagOpts(std::string flag) const
	{
		static std::list<std::string> bad;
		auto it = m_Options.find(flag);
		if (it != m_Options.end()) return it->second;
		return bad;
	}

	CCLP(const std::vector<const char*> & args)
	{
		m_CommandLine = args;
	}
};