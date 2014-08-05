#pragma once

#include "pugixml\pugixml.hpp"

namespace SCRambl
{
	/*class XML
	{
		template <typename It>
		class Range
		{
		public:
			typedef It iterator;
			typedef const It const_iterator;
		};
		class Node
		{
			friend class XML;
			pugi::xml_node			m_Node;

			Node(pugi::xml_node node) : m_Node(node)
			{
				node.children();
			}
			inline Node & operator=(const pugi::xml_node & node)			{ m_Node = node; return *this; }

		public:
			Node() = default;

			inline Node Child(const char * name) const				{ return m_Node.child(name); }
			inline Node operator[](const char * name) const			{ return Child(name); }
			inline Node operator[](const std::string& name) const	{ return Child(name.c_str()); }
			inline std::iterator begin() const { return _begin; }
		It end() const { return _end; }

			inline operator bool() const						{ return m_Node; }
		};

	private:
		pugi::xml_document		m_Document;
		pugi::xml_parse_result	m_Result;

	public:
		XML(const char * szFilePath) : m_Result(m_Document.load_file(szFilePath))
		{
		}

		inline Node Child(const char * name) const				{ return m_Document.child(name); }
		inline Node operator[](const char * name) const			{ return Child(name); }
		inline Node operator[](const std::string& name) const	{ return Child(name.c_str()); }

		inline operator bool() const {
			return m_Result.status == pugi::xml_parse_status::status_ok;
		}
	};*/
}