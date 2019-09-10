#pragma once
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include "traits/editor_traits.h"
#include "traits/xml_traits.h"

namespace filter
{
namespace hml
{
	class filter_t
	{
	public:
		typedef xml_traits::xml_document_t xml_document_t;
		typedef xml_traits::path_t path_t;
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::sections_t sections_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rules_t rules_t;

		filter_t();

		std::unique_ptr<xml_document_t> open(const std::string& path);
		void save(const std::string& path, std::unique_ptr<xml_document_t>& document);

		sections_t extract_all_texts(std::unique_ptr<xml_document_t>& document);
		void replace_privacy(const rules_t& rules, char16_t replacement, std::unique_ptr<xml_document_t>& document);
		sections_t search_privacy(const rules_t& rules, std::unique_ptr<xml_document_t>& document);
	private:
		std::string text_tag_name() const;
		std::string para_tag_name() const;
	};
}
}