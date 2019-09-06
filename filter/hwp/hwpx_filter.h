#pragma once
#include <iostream>
#include <memory>
#include <map>
#include <string>
#include "traits/xml_traits.h"

namespace filter
{
namespace hwpx
{
	class filter_t
	{
	public:
		typedef xml_traits::xml_document_t xml_document_t;
		typedef xml_traits::path_t path_t;
		typedef xml_traits::izstream_t izstream_t;
		typedef xml_traits::ozstream_t ozstream_t;

		typedef std::wstring para_t;
		typedef std::vector<para_t> section_t;
		typedef std::vector<section_t> sections_t;
		filter_t();
		sections_t extract_all_texts(const std::string& path);
	private:
	};

	class consumer_t
	{
	public:
		typedef xml_traits::xml_document_t xml_document_t;
		typedef xml_traits::path_t path_t;
		typedef xml_traits::izstream_t izstream_t;

		typedef std::map< std::string, std::unique_ptr<xml_document_t> > part_documents_t;
		typedef std::vector<path_t> part_names_t;
		consumer_t();
		void open(const path_t& path);
		std::unique_ptr<izstream_t> open_package(const path_t& path);
		std::unique_ptr<xml_document_t> extract_part(const path_t& path, std::unique_ptr<izstream_t>& izstream);

		part_documents_t& get_parts() {
			return parts;
		}
		part_names_t& get_names() {
			return &names;
		}
	private:
		void load_part(const path_t& path, std::unique_ptr<izstream_t>& izstream);

		part_documents_t parts;
		part_names_t names;
	};

	class producer_t
	{
	public:
		typedef xml_traits::xml_document_t xml_document_t;
		typedef xml_traits::path_t path_t;
		typedef xml_traits::ozstream_t ozstream_t;
		producer_t();
	private:
	};
}
}