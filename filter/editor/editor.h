#pragma once
#include <string>
#include <memory>
#include "traits/editor_traits.h"
#include "traits/xml_traits.h"

namespace filter
{
namespace xml
{
	class extract_texts_t;
	class editor_t
	{
	public:
		typedef xml_traits::xml_document_t xml_document_t;
		typedef xml_traits::path_t path_t;
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::sections_t sections_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rules_t rules_t;

		editor_t();
		editor_t& extract(const std::string& text_tag, const std::string& para_tag);
		editor_t& find(const rules_t& rules);
		editor_t& replace(char16_t replacement = u'*');
		editor_t& finalize(std::unique_ptr<xml_document_t>& document);

		section_t get_extract_result();
		sections_t get_find_result();
	private:
		std::unique_ptr<extract_texts_t> strategy;
		std::string text_tag;
		std::string para_tag;
		rules_t rules;
		char16_t replacement;
	};
}
}