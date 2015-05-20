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
	class XMLRange;
	class XMLNodeIterator;

	enum XMLValueType {
		xmlvalue_null, xmlvalue_string, xmlvalue_numeric, xmlvalue_version
	};

	class XMLValue
	{
		std::string m_str;

	public:
		using Vector = std::vector<XMLValue>;

		XMLValue();					// null-value
		XMLValue(std::string);
		XMLValue(const char*);
		XMLValue(pugi::xml_text);
		
		template<typename T> auto AsNumber(T default_value) const->T;
		/*template<> auto AsNumber(int) const->int;	
		template<> auto AsNumber(long) const->long;
		template<> auto AsNumber(unsigned long) const->unsigned long;
		template<> auto AsNumber(long long) const->long long;
		template<> auto AsNumber(unsigned long long) const->unsigned long long;
		template<> auto AsNumber(float) const->float;
		template<> auto AsNumber(double) const->double;*/
		template<typename T> inline auto AsNumber() const->T { return AsNumber<T>(0); }

		auto AsString(const std::string& default_value = "") const->const std::string&;
		auto AsVersion(VersionStruct default_value = 0) const->VersionStruct;
		auto AsBool(bool default_value = false) const->bool;
		auto AsList(char sep = ',', char delim = '"') const->Vector;

		auto Raw() const->std::string;

		auto IsValidVar() const->bool;
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
		auto GetPugi() const->const decltype(m_attr)&;
		auto GetPugi()->decltype(m_attr)&;
	};

	class XMLNode
	{
		pugi::xml_node m_node;

	public:
		typedef XMLNodeIterator Iterator;

		XMLNode() = default;
		XMLNode(const XMLNode&) = default;
		XMLNode(const decltype(m_node)& node);
		XMLNode(pugi::xml_node_struct* node);
		operator bool() const;

		auto Begin() const->Iterator;
		auto End() const->Iterator;
		auto begin() const->Iterator;
		auto end() const->Iterator;
		auto Name() const->std::string;
		auto Children() const->XMLRange;
		auto GetNode(std::wstring) const->XMLNode;
		auto GetNode(std::string) const->XMLNode;
		auto GetNode(const wchar_t*) const->XMLNode;
		auto GetNode(const char*) const->XMLNode;
		auto GetValue() const->XMLValue;
		auto GetAttribute(std::string) const->XMLAttribute;
		auto GetAttribute(const char*) const->XMLAttribute;
		auto GetPugi() const->const decltype(m_node)&;
		auto GetPugi()->decltype(m_node)&;
	};

	class XMLNodeIterator
	{
		pugi::xml_node::iterator m_it;
		mutable XMLNode m_node;

	public:
		XMLNodeIterator() = default;
		XMLNodeIterator(const XMLNodeIterator&) = default;
		XMLNodeIterator(pugi::xml_node::iterator);

		XMLNode operator*() const;
		XMLNode* operator->() const;			// /fu
		XMLNodeIterator operator++(int);
		XMLNodeIterator operator--(int);
		const XMLNodeIterator& operator++();
		const XMLNodeIterator& operator--();
		bool operator==(const XMLNodeIterator& rhs) const;
		bool operator!=(const XMLNodeIterator& rhs) const;
	};

	class XMLRange
	{
		pugi::xml_object_range<XMLNodeIterator> m_range;

	public:
		XMLRange(XMLNodeIterator beg, XMLNodeIterator end) : m_range(beg, end)
		{ }
		inline XMLNodeIterator Begin() const { return m_range.begin(); }
		inline XMLNodeIterator End() const { return m_range.end(); }
		inline XMLNodeIterator begin() const { return Begin(); }
		inline XMLNodeIterator end() const { return End(); }
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
		XML(std::string path);
		XML(std::wstring path);
		XML(const char* path);
		XML(const wchar_t* path);
		XML(const XML&) = default;
		operator bool() const;
		
		auto Children() const->XMLRange;
		auto Node() const->XMLNode;
		auto GetNode(std::string) const->XMLNode;
		auto GetNode(std::wstring) const->XMLNode;
		auto GetNode(const char*) const->XMLNode;
		auto GetNode(const wchar_t*) const->XMLNode;

		auto GetParseResult() const->decltype(m_parseData);
		auto GetPugiParseResult() const->decltype(m_parseResult);
		auto GetPugi() const->const decltype(m_doc)&;
		auto GetPugi()->decltype(m_doc)&;
	};
}