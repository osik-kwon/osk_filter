#pragma once
#include <string>
#include <memory>
#include <fstream>

#include <traits/xml_traits.h>
#include <pole/pole.h>
#include <xml/parser>
#include <xlnt/utils/path.hpp>
#include <xlnt/detail/serialization/zstream.hpp>

namespace filter
{
namespace signature
{
	class range_t
	{
	public:
		range_t();
		range_t(const std::string& buffer, std::size_t begin, std::size_t end);
		bool match(const std::string& rule) const;
		bool equal(const std::string& dest) const;
	private:
		std::size_t begin;
		std::size_t end;
		std::string data;
	};

	class attribute_t
	{
	public:
		attribute_t(const std::unique_ptr<xml::parser>& parser, const std::string& name);
		bool equal(const std::string& dest) const;
		bool match(const std::string& rule) const;
		bool exist() const;
	private:
		const std::unique_ptr<xml::parser>& parser;
		const std::string& name;
	};

	class element_t
	{
	public:
		element_t(const std::unique_ptr<xml::parser>& parser);
		bool equal(const std::string& dest) const;
		bool exist() const;
		attribute_t attribute(const std::string& name) const;
	private:
		const std::unique_ptr<xml::parser>& parser;
	};

	class sequence_t
	{
	public:
		sequence_t();
		sequence_t(std::unique_ptr<std::streambuf>& buf, const std::string& path, size_t nth_element);
		sequence_t(const std::string& path, size_t nth_element);
		element_t element(const std::string& name) const;
		attribute_t attribute(const std::string& name) const;
	private:
		void load(const std::string& path);
		void load(std::unique_ptr<std::streambuf>& buf, const std::string& path);
		void visit(size_t nth_element);

		std::unique_ptr<std::ifstream> file;
		std::unique_ptr<xml::parser> parser;
		std::unique_ptr<std::istream> stream;
	};

	class attributes_t
	{
	public:
		typedef xml_traits::xml_document_t xml_document_t;
		attributes_t(const pugi::xpath_node_set& nodes, const std::string& name);
		bool equal(const std::string& dest) const;
		bool match(const std::string& rule) const;
		bool exist() const;
	private:
		const pugi::xpath_node_set& nodes;
		const std::string& name;
	};

	class xpath_t
	{
	public:
		typedef xml_traits::xml_document_t xml_document_t;
		xpath_t();
		xpath_t(const std::string& path, const std::string& xpath);
		xpath_t(std::unique_ptr<std::streambuf>& stream, const std::string& xpath);
		bool exist() const;
		attributes_t attributes(const std::string& name);
	private:
		std::string xpath;
		std::unique_ptr<xml_document_t> document;
	};

	class package_t
	{
	public:
		typedef xlnt::path path_t;
		typedef xlnt::detail::izstream izstream_t;
		typedef xlnt::detail::ozstream ozstream_t;
		package_t();
		package_t(const std::string& path, const std::string& part_name);
		sequence_t& sequence(size_t nth_element);
		xpath_t& xpath(const std::string& path);
	private:
		void open_package();
		std::string path;
		std::string part_name;
		sequence_t sequence_buffer;
		xpath_t xpath_buffer;

		std::unique_ptr<std::ifstream> source;
		std::unique_ptr<izstream_t> archive;
	};

	class compound_t
	{
	public:
		typedef POLE::Storage storage_t;
		compound_t();
		compound_t(const std::string& path, const std::string& stream_name);
		range_t& range(size_t begin, size_t end);
	private:
		void open_storage();
		std::string path;
		std::string stream_name;
		std::unique_ptr<range_t> range_buffer;
		std::unique_ptr<storage_t> storage;
	};

	class storage_t
	{
	public:
		storage_t(const std::string& path);
		std::string read_header(size_t minimal_length);
		bool empty() const;
		range_t& range(size_t begin, size_t end);
		sequence_t& sequence(size_t nth_element);
		package_t& package(const std::string& part_name);
		compound_t& compound(const std::string& stream_name);
		const std::string& get_header() const;
	private:
		std::string path;
		std::string header;

		std::unique_ptr<range_t> range_buffer;
		std::unique_ptr<sequence_t> sequence_buffer;
		std::unique_ptr<package_t> package_buffer;
		std::unique_ptr<compound_t> compound_buffer;
	};
}
}