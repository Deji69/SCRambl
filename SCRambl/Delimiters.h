/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "utils.h"
#include "Configuration.h"

namespace SCRambl
{
	class Engine;
	
	class Delimiter
	{
	public:
		Delimiter(int type, std::string beg, std::string end) : m_type(type), m_open(beg), m_close(end)
		{ }

		inline int Type() const						{ return m_type; }
		inline const std::string& Open() const		{ return m_open; }
		inline const std::string& Close() const		{ return m_close; }

	private:
		int m_type;
		std::string m_open;
		std::string m_close;
	};

	class Delimiters
	{
	public:
		Delimiters();
		Delimiters(Engine& eng);

		void AddDelimiterType(std::string);

		/*inline const Delimiter<T>& AddDelimiter(T type, std::string beg, std::string end) {
			m_delimiters.emplace_back(type, beg, end);
			auto* delim = &m_delimiters.back();
			m_openingDelims.emplace(beg, delim);
			m_closingDelims.emplace(end, delim);
		}
		inline const Delimiter<T>* FindClosing(std::string s) {
			auto it = std::find(std::begin(m_openingDelims), std::end(m_openingDelims), s);
			if (it != std::end(m_openingDelims))
				return *it->second;
			return nullptr;
		}
		inline const Delimiter<T>* FindClosing(std::string s) {
			auto it = std::find(std::begin(m_closingDelims), std::end(m_closingDelims), s);
			if (it != std::end(m_closingDelims))
				return *it->second;
			return nullptr;
		}*/

	private:
		std::shared_ptr<Configuration> m_config;
		std::vector<Delimiter> m_delimiters;
		std::unordered_map<std::string, Delimiter *> m_openingDelims;
		std::unordered_map<std::string, Delimiter *> m_closingDelims;
	};
}