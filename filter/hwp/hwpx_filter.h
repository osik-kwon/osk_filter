#pragma once
#include <iostream>
#include <memory>
#include <map>
#include <regex>
#include <string>
#include "traits/xml_traits.h"

namespace filter
{
namespace hwpx
{
	class consumer_t;
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
		std::unique_ptr<consumer_t> open(const std::string& path);
		void save(const std::string& path, std::unique_ptr<consumer_t>& consumer);
	private:
		std::regex section_name_regex() const;
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

		xml_document_t* get_part(const path_t& path)
		{
			auto part = parts.find(path.string());
			if (part != parts.end())
				return part->second.get();
			return nullptr;
		}

		part_documents_t& get_parts() {
			return parts;
		}
		part_names_t& get_names() {
			return names;
		}
	private:
		void load_part(const path_t& path, std::unique_ptr<izstream_t>& izstream);

		std::ifstream source;
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
		void save(const path_t& path, std::unique_ptr<consumer_t>& consumer);
	private:
		std::ofstream dest;
	};
}
}