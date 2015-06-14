#include "stdafx.h"
#include "XML.h"
#include "Engine.h"

namespace SCRambl
{
	inline XMLResult MakeXMLResult(XMLStatus status, XMLError err = xmlerror_none) {
		XMLResult res;
		res.status = status;
		res.error = err;
		return res;
	}
	std::string ProcessXMLValue(std::string str) {
		std::string v;
		for (auto it = str.begin(); it != str.end(); ++it) {
			if (*it == '(') {
				auto beg = it;
				if (*++it == '$') {
					++it;
					auto name_beg = it;
					while (std::isalnum(*it)) ++it;
					std::string name(name_beg, it);
					if (*++it == ')') {
						BREAK();
					}
					else it = beg;
					name.size();
				}
				else it = beg;
			}
			
			v += *it;
		}
		return v;
	}
	XMLValueType GetXMLValueType(std::string str) {
		if (!str.empty()) {
			bool couldBeNumber = true,
				couldBeVersion = true;
			bool startsWithNumber = false,
				startsWithLetter = false,
				startsWithPeriod = false;
			int numPeriods = 0, step = 0;
			for (auto c : str) {
				if (c == '.') {
					if (!step) {
						startsWithPeriod = true;
						couldBeVersion = false;
					}
					++numPeriods;
				}
				else if (isdigit(c)) {
					if (!step) startsWithNumber = true;
				}

				++step;
			}
			if (numPeriods) {
				couldBeNumber = false;
				if (numPeriods > 4) couldBeVersion = false;
			}
		}
		return xmlvalue_null;
	}

	void ParseXML(XML& xml, XMLParseData& data) {
		if (xml) {
			data.SCRamblNode = xml.GetPugi().child("SCRambl");
			if (data.SCRamblNode) {
				auto attr = data.SCRamblNode.GetAttribute("Version");
				auto val = attr.GetValue();
				if (val.IsValidVersion()) {
					data.SCRamblVersion = val.AsVersion();
					attr = data.SCRamblNode.GetAttribute("FileVersion");
					val = attr.GetValue();
					if (val.IsValidVersion()) data.FileVersion = val.AsVersion();
					else data.Result = MakeXMLResult(xmlstatus_error, xmlerror_bad_file_version);
				}
				else data.Result = MakeXMLResult(xmlstatus_error, xmlerror_bad_scrambl_version);
			}
			else data.Result = MakeXMLResult(xmlstatus_error, xmlerror_no_scrambl_node);
		}
		else data.Result = MakeXMLResult(xmlstatus_error, xmlerror_parse_failed);
	}

	// -- Take care of prior commitments... --

	/* XML */
	auto XML::GetNode(const char * name) const->XMLNode {
		return m_parseData.Result ? m_parseData.SCRamblNode.GetPugi().child(name) : XMLNode();
	}
	auto XML::GetNode(std::string name) const->XMLNode { return GetNode(name.c_str()); }
	auto XML::GetNode(const wchar_t* name) const->XMLNode { return GetNode(narrow(name)); }
	auto XML::GetNode(std::wstring name) const->XMLNode { return GetNode(narrow(name)); }
	auto XML::Children() const->XMLRange { return m_parseData.SCRamblNode.Children(); }
	auto XML::Node() const->XMLNode { return m_parseData.SCRamblNode; }
	auto XML::GetParseResult() const->decltype(m_parseData) { return m_parseData; }
	auto XML::GetPugiParseResult() const->decltype(m_parseResult) { return m_parseResult; }
	auto XML::GetPugi() const->const decltype(m_doc)& { return m_doc; }
	auto XML::GetPugi()->decltype(m_doc)& { return m_doc; }
	XML::XML() { ParseXML(*this, m_parseData); }
	XML::XML(std::string path) : XML(widen(path).c_str()) { ParseXML(*this, m_parseData); }
	XML::XML(std::wstring path) : XML(path.c_str()) { ParseXML(*this, m_parseData); }
	XML::XML(const char* path) : m_parseResult(m_doc.load_file(widen(path).c_str())) { ParseXML(*this, m_parseData); }
	XML::XML(const wchar_t* path) : m_parseResult(m_doc.load_file(path)) { ParseXML(*this, m_parseData); }
	XML::operator bool() const { return m_parseResult; }
	/* XMLValue */
	template<typename T, typename F>
	T sto(std::string str, T default_val, F func = [](const std::string& str){ return std::stoi(str); }) {
		T v;
		try { v = func(str); }
		catch (...) { v = default_val; }
		return v;
	}
	VersionStruct stoversion(std::string str) {
		if (!str.empty()) {
			VersionStruct ver;
			size_t dotPos = 0;

			for (int c = 4; c && dotPos != str.npos; --c) {
				size_t nextDotPos = str.find_first_of('.', dotPos);
				unsigned long i;
				if (nextDotPos != str.npos) {
					size_t n;
					i = std::stoul(str.substr(dotPos, nextDotPos - dotPos), &n, 0);
					dotPos += n;
					if (dotPos < nextDotPos) throw;
					++dotPos;
				}
				else {
					size_t n;
					i = std::stoul(str.substr(dotPos, str.length() - dotPos), &n, 0);
					dotPos += n;
					if (dotPos < str.length()) throw;
					dotPos = str.npos;
				}

				if (i > 0xFF) throw;
				if (c == 4) ver.Major = i;
				else if (c == 3) ver.Minor = i;
				else if (c == 2) ver.Beta = i;
				else ver.Alpha = i;
			}
			return ver;
		}
		throw;
	}
	auto XMLValue::IsValidNumber() const->bool {
		bool valid = true;
		try {
			std::stoll(m_str);
		}
		catch (...) {
			try {
				std::stod(m_str);
			}
			catch (...) {
				try {
					std::stoull(m_str);
				}
				catch (...) {
					valid = false;
				}
			}
		}
		return valid;
	}
	auto XMLValue::IsValidVersion() const->bool {
		if (!m_str.empty()) {
			try {
				stoversion(m_str);
				return true;
			}
			catch (...) {}
		}
		return false;
	}
	auto XMLValue::IsValidBool() const->bool {
		if (!m_str.empty()) {
			switch (m_str[0]) {
			case 't': case 'T':
			case 'y': case 'Y':
			case '1':
			case 'f': case 'F':
			case 'n': case 'N':
			case '0':
				return true;
			}
		}
		return false;
	}
	auto XMLValue::IsEmpty() const->bool {
		return m_str.empty();
	}
	auto XMLValue::Size() const->size_t {
		return m_str.size();
	}
	template<> auto XMLValue::AsNumber(int v) const->int { return sto<int>(m_str, v, [](std::string str){ return std::stoi(str); }); }
	template<> auto XMLValue::AsNumber(long v) const->long { return sto<long>(m_str, v, [](std::string str){ return std::stol(str, nullptr, 0); }); }
	template<> auto XMLValue::AsNumber(unsigned int v) const->unsigned int { return sto<unsigned long>(m_str, v, [](std::string str){ return std::stoul(str, nullptr, 0); }); }
	template<> auto XMLValue::AsNumber(unsigned long v) const->unsigned long { return sto<unsigned long>(m_str, v, [](std::string str){ return std::stoul(str, nullptr, 0); }); }
	template<> auto XMLValue::AsNumber(long long v) const->long long { return sto<long long>(m_str, v, [](std::string str){ return std::stoll(str, nullptr, 0); }); }
	template<> auto XMLValue::AsNumber(unsigned long long v) const->unsigned long long { return sto<unsigned long long>(m_str, v, [](std::string str){ return std::stoull(str, nullptr, 0); }); }
	template<> auto XMLValue::AsNumber(float v) const->float { return sto<float>(m_str, v, [](std::string str){ return std::stof(str); }); }
	template<> auto XMLValue::AsNumber(double v) const->double { return sto<double>(m_str, v, [](std::string str){ return std::stod(str); }); }
	auto XMLValue::AsBool(bool default_value) const->bool{
		if (!m_str.empty()) {
			// check for special stuff...
			auto str = tolower(m_str);
			if (str == "on" || str == "enable")
				return true;
			else if (str == "off" || str == "disable")
				return false;

			switch (str[0]) {
			case 't': case 'y': case '1':
				return true;
			case 'f': case 'n': case '0':
				return false;
			}
		}
		return default_value;
	}
	auto XMLValue::AsVersion(VersionStruct default_value) const->VersionStruct{
		try {
			VersionStruct ver = stoversion(m_str);
			return ver;
		}
		catch (...) { }
		return default_value;
	}
	auto XMLValue::AsList(char sep, char delim) const->Vector {
		Vector vec;
		std::string str;
		bool hasDelim = false;
		for (auto it = m_str.begin(); it != m_str.end(); ++it) {
			if (!hasDelim && *it == sep) {
				vec.emplace_back(str);
				str = "";
			}
			else {
				if (*it == delim) {
					if (hasDelim)
						hasDelim = false;
					else if (str.empty() && *it == delim)
						hasDelim = true;
				}
				str += *it;
			}
		}
		if (!str.empty()) vec.emplace_back(str);
		return vec;
	}
	auto XMLValue::AsString(const std::string& default_value) const->const std::string& {
		return m_str.empty() ? default_value : m_str;
	}
	auto XMLValue::Raw() const->std::string { return m_str; }
	XMLValue::operator bool() const { return !IsEmpty(); }
	XMLValue::XMLValue() { }
	XMLValue::XMLValue(std::string str) : m_str(str) { }
	XMLValue::XMLValue(const char* str) : m_str(str) { }
	XMLValue::XMLValue(pugi::xml_text txt) : m_str(txt.as_string()) { }
	/* XMLAttribute */
	auto XMLAttribute::GetValue() const->XMLValue& {
		m_value = m_attr.as_string();
		return m_value;
	}
	auto XMLAttribute::GetPugi()->decltype(m_attr)& { return m_attr; }
	XMLAttribute::XMLAttribute(const decltype(m_attr)& attr) : m_attr(attr) { }
	XMLAttribute::XMLAttribute(pugi::xml_attribute_struct* attr) : m_attr(attr) { }
	XMLValue& XMLAttribute::operator*() const { return GetValue(); }
	XMLValue* XMLAttribute::operator->() const { return &GetValue(); }
	XMLAttribute::operator bool() const { return !m_attr.empty(); }
	/* XMLNode */
	auto XMLNode::GetNode(const char * name) const->XMLNode { return m_node ? m_node.child(name) : XMLNode(); }
	auto XMLNode::GetNode(std::string name) const->XMLNode { return GetNode(name.c_str()); }
	auto XMLNode::GetNode(const wchar_t * name) const->XMLNode { return GetNode(narrow(name)); }
	auto XMLNode::GetNode(std::wstring name) const->XMLNode { return GetNode(name.c_str()); }
	auto XMLNode::GetValue() const->XMLValue& {
		m_value = m_node.text();
		return m_value;
	}
	auto XMLNode::GetAttribute(const char * attr) const->XMLAttribute& {
		m_attr = m_node.attribute(attr);
		return m_attr;
	}
	auto XMLNode::GetAttribute(std::string attr) const->XMLAttribute& { return GetAttribute(attr.c_str()); }
	auto XMLNode::GetPugi() const->const decltype(m_node)& { return m_node; }
	auto XMLNode::GetPugi()->decltype(m_node)& { return m_node; }
	auto XMLNode::Children() const->XMLRange {
		auto children = m_node.children();
		return XMLRange(children.begin(), children.end());
	}
	auto XMLNode::Name() const->std::string { return m_node.name(); }
	auto XMLNode::Begin() const->Iterator { return m_node.begin(); }
	auto XMLNode::End() const->Iterator { return m_node.end(); }
	auto XMLNode::begin() const->Iterator { return Begin(); }
	auto XMLNode::end() const->Iterator { return End(); }
	XMLNode::XMLNode(const pugi::xml_node& node) : m_node(node) { }
	XMLNode::XMLNode(pugi::xml_node_struct* node) : m_node(node) { }
	XMLValue& XMLNode::operator*() const {
		return GetValue();
	}
	XMLValue* XMLNode::operator->() const {
		return &GetValue();
	}
	XMLAttribute& XMLNode::operator[](std::string i) const { return GetAttribute(i); }
	XMLAttribute& XMLNode::operator[](const char* i) const { return GetAttribute(i); }
	XMLNode::operator bool() const { return !m_node.empty(); }
	/* XMLNodeIterator */
	bool XMLNodeIterator::operator==(const XMLNodeIterator& rhs) const { return m_it == rhs.m_it; }
	bool XMLNodeIterator::operator!=(const XMLNodeIterator& rhs) const { return !(*this == rhs); }
	const XMLNodeIterator& XMLNodeIterator::operator++() {
		++m_it;
		return *this;
	}
	const XMLNodeIterator& XMLNodeIterator::operator--() {
		--m_it;
		return *this;
	}
	XMLNodeIterator XMLNodeIterator::operator++(int) {
		XMLNodeIterator v(*this);
		++(*this);
		return v;
	}
	XMLNodeIterator XMLNodeIterator::operator--(int) {
		XMLNodeIterator v(*this);
		--(*this);
		return v;
	}
	XMLNode* XMLNodeIterator::operator->() const {
		m_node = XMLNode(*m_it);
		return &m_node;
	}
	XMLNode XMLNodeIterator::operator*() const { return m_it.operator*(); }
	XMLNodeIterator::XMLNodeIterator(pugi::xml_node::iterator it) : m_it(it) { }
	/* XMLParseData */
	XMLParseData::XMLParseData() { }
	/* XMLResult */
	XMLResult::XMLResult() : status(xmlstatus_ok), error(xmlerror_none) { }
	XMLResult::operator bool() const { return status == xmlstatus_ok; }
}