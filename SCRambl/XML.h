/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include "utils.h"

namespace SCRambl
{
	class XMLNode
	{
		pugi::xml_node m_node;

	public:
		XMLNode() = default;
		XMLNode(const XMLNode&) = default;
		XMLNode(const pugi::xml_node& node) : m_node(node) { }
		XMLNode(pugi::xml_node_struct* node) : m_node(node) { }

		auto GetPugi() -> decltype(m_node)							{ return m_node; }
	};

	class XML
	{
		pugi::xml_document m_doc;
		pugi::xml_parse_result m_parseResult;

	public:
		XML() = default;
		XML(const XML&) = default;
		XML(std::string path) : XML(path.c_str()) { }
		XML(std::wstring path) : XML(path.c_str()) { }
		XML(const char* path) : m_parseResult(m_doc.load_file(path)) { }
		XML(const wchar_t* path) : m_parseResult(m_doc.load_file(path)) { }

		auto GetParseResult() -> decltype(m_parseResult)			{ return m_parseResult; }
		auto GetPugi() -> decltype(m_doc)&							{ return m_doc; }
	};
}