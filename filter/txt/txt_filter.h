#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "traits/editor_traits.h"
#include "txt/txt_document.h"

namespace filter
{
namespace txt
{
	class filter_t
	{
	public:
		typedef consumer_t::char_t char_t;
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::sections_t sections_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rules_t rules_t;

		filter_t();
		std::string detect_charset(const std::string& path);
		std::unique_ptr<consumer_t> open(const std::string& path);
		void save(const std::string& path, std::unique_ptr<consumer_t>& consumer, std::string dest_charset = "");
		sections_t extract_all_texts(std::unique_ptr<consumer_t>& consumer);
		sections_t search_privacy(const rules_t& rules, std::unique_ptr<consumer_t>& consumer);
		void replace_privacy(const rules_t& rules, char16_t replacement, std::unique_ptr<consumer_t>& consumer);
	private:
	};
}
}