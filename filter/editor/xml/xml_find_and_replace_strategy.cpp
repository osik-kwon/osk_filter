#include "filter_pch.h"
#include "editor/xml/xml_find_and_replace_strategy.h"
#include "editor/xml/xml_replace_texts.h"

namespace filter
{
namespace xml
{
	void find_and_replace_strategy_t::do_nothing(const std::string& text, pugi::xml_node& node, std::vector<search_texts_t>& rules)
	{}

	void find_and_replace_strategy_t::find_only(const std::string& text, pugi::xml_node& node, std::vector<search_texts_t>& rules)
	{
		for (auto& rule : rules)
			rule.search(text, node);
	}

	void find_and_replace_strategy_t::find_and_replace(const std::string& text, pugi::xml_node& node, std::vector<search_texts_t>& rules)
	{
		for (auto& rule : rules)
		{
			rule.search(text, node);
			if (!rule.empty())
			{
				replace_texts_t::replace(text, node, rule.get_pattern(), rule.get_replacement());
			}
		}
	}
}
}