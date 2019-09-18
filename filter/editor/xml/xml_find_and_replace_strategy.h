#pragma once
#include "traits/xml_traits.h"
#include "editor/xml/xml_search_texts.h"

namespace filter
{
namespace xml
{
	struct find_and_replace_strategy_t
	{
		static void do_nothing(const std::string& text, pugi::xml_node& node, std::vector<search_texts_t>& rules);
		static void find_only(const std::string& text, pugi::xml_node& node, std::vector<search_texts_t>& rules);
		static void find_and_replace(const std::string& text, pugi::xml_node& node, std::vector<search_texts_t>& rules);
	};
}
}