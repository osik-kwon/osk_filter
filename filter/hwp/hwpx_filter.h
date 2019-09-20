#pragma once
#include <iostream>
#include <memory>
#include <map>
#include <regex>
#include <string>
#include "traits/xml_traits.h"
#include "traits/editor_traits.h"
#include "io/open_package_conventions.h"

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
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::sections_t sections_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rules_t rules_t;

		filter_t();
		std::unique_ptr<consumer_t> open(const std::string& path);
		void save(const std::string& path, std::unique_ptr<consumer_t>& consumer);
		sections_t extract_all_texts(std::unique_ptr<consumer_t>& consumer);
		sections_t search_privacy(const rules_t& rules, std::unique_ptr<consumer_t>& consumer);
		void replace_privacy(const rules_t& rules, char16_t replacement, std::unique_ptr<consumer_t>& consumer);
	private:
		std::string text_tag_name() const;
		std::string para_tag_name() const;
		std::regex section_name_regex() const;
	};
}
}