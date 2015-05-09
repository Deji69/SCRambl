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
	enum XMLValueType {
		xmlvalue_null, xmlvalue_string, xmlvalue_numeric, xmlvalue_version
	};

	class XMLValue
	{
		XMLValueType m_type;
		std::string m_str;

	public:
		using Vector = std::vector<XMLValue>;

		XMLValue();					// null-value
		XMLValue(std::string);
		
		template<typename T> auto AsNumber(T default_value) const->T;
		template<> auto AsNumber(int) const->int;
		template<> auto AsNumber(long) const->long;
		template<> auto AsNumber(unsigned long) const->unsigned long;
		template<> auto AsNumber(long long) const->long long;
		template<> auto AsNumber(unsigned long long) const->unsigned long long;
		template<> auto AsNumber(float) const->float;
		template<> auto AsNumber(double) const->double;
		template<typename T> inline auto AsNumber() const->T { return AsNumber<T>(0); }

		auto AsString(std::string default_value = "") const->std::string;
		auto AsVersion(VersionStruct default_value = 0) const->VersionStruct;
		auto AsBool(bool default_value = false) const->bool;
		auto AsList(char sep = ',', char delim = '"') const->Vector;

		auto IsValidNumber() const->bool;
		auto IsValidVersion() const->bool;
		auto IsValidBool() const->bool;
	};

	class XMLAttribute
	{
		pugi::xml_attribute m_attr;

	public:
		XMLAttribute() = default;
		XMLAttribute(const XMLAttribute&) = default;
		XMLAttribute(const decltype(m_attr)& attr);
		XMLAttribute(pugi::xml_attribute_struct* attr);
		operator bool() const;

		auto GetValue() const->XMLValue;
		auto GetPugi()->decltype(m_attr)&;
	};

	class XMLNode
	{
		pugi::xml_node m_node;

	public:
		XMLNode() = default;
		XMLNode(const XMLNode&) = default;
		XMLNode(const decltype(m_node)& node);
		XMLNode(pugi::xml_node_struct* node);
		operator bool() const;

		auto GetAttribute(std::string) const->XMLAttribute;
		auto GetAttribute(const char *) const->XMLAttribute;
		auto GetPugi()->decltype(m_node)&;
	};

	enum XMLStatus {
		// main states
		xmlstatus_ok,
		xmlstatus_error,
	};
	enum XMLError {
		// error states
		xmlerror_none,
		xmlerror_parse_failed,						// pugi xml parsing failed
		xmlerror_no_scrambl_node,					// the required root node <SCRambl> is missing
		xmlerror_bad_scrambl_version,				// the Version attribute of the <SCRambl> node is missing or invalid
		xmlerror_bad_file_version,					// the FileVersion attribute of the <SCRambl> node is missing or invalid
		xmlerror_superior_version,					// the xml file was made for a higher version of SCRambl
	};
	struct XMLResult {
		XMLStatus status;
		XMLError error;

		XMLResult();
		operator bool() const;
	};

	struct XMLParseData {
		XMLResult Result;
		VersionStruct SCRamblVersion;
		VersionStruct FileVersion;
		XMLNode SCRamblNode;

		XMLParseData();
	};
	
	class XML
	{
		pugi::xml_document m_doc;
		pugi::xml_parse_result m_parseResult;
		XMLParseData m_parseData;
		
	public:
		XML();
		XML(const XML&) = default;
		XML(std::string path);
		XML(std::wstring path);
		XML(const char* path);
		XML(const wchar_t* path);
		operator bool() const;

		auto GetPugiParseResult()->decltype(m_parseResult);
		auto GetPugi()->decltype(m_doc)&;
	};
}