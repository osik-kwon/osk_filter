#pragma once
#include <iostream>
#include <memory>
#include <map>
#include <regex>
#include <string>
#include "traits/xml_traits.h"

namespace filter {
namespace opc {
	class consumer_t;
	class producer_t;
}}

namespace filter
{
namespace hwpx
{
	using ::filter::opc::consumer_t;
	using ::filter::opc::producer_t;
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
		std::unique_ptr<consumer_t> open(const std::string& path);
		void save(const std::string& path, std::unique_ptr<consumer_t>& consumer);

		sections_t extract_all_texts(std::unique_ptr<consumer_t>& consumer);
		void replace_privacy(const std::wregex& pattern, char16_t replace_dest, std::unique_ptr<consumer_t>& consumer);
	private:
		std::regex section_name_regex() const;
	};
}
}