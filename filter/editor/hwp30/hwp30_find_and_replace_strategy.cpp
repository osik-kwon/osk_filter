#include "filter_pch.h"
#include "editor/hwp30/hwp30_find_and_replace_strategy.h"
#include "editor/hwp30/hwp30_replace_texts.h"

namespace filter
{
namespace hwp30
{
	void find_and_replace_strategy_t::do_nothing(rule_string& texts, para_ref_t& para_ref, std::vector<search_texts_t>& rules)
	{}

	void find_and_replace_strategy_t::find_only(rule_string& texts, para_ref_t& para_ref, std::vector<search_texts_t>& rules)
	{
		for (auto& rule : rules)
			rule.search(texts, para_ref);
	}

	void find_and_replace_strategy_t::find_and_replace(rule_string& texts, para_ref_t& para_ref, std::vector<search_texts_t>& rules)
	{
		for (auto& rule : rules)
		{
			rule.search(texts, para_ref);
			if (!rule.empty())
			{
				replace_texts_t::replace(texts, para_ref, rule.get_pattern(), rule.get_replacement());
			}
		}
	}
}
}