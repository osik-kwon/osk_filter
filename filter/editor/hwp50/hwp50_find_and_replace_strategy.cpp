#include "filter_pch.h"
#include "editor/hwp50/hwp50_find_and_replace_strategy.h"
#include "editor/hwp50/hwp50_replace_texts.h"

namespace filter
{
namespace hwp50
{
	void find_and_replace_strategy_t::do_nothing(rule_string& texts, std::reference_wrapper<record_t>& record, std::vector<search_texts_t>& rules)
	{}

	void find_and_replace_strategy_t::find_only(rule_string& texts, std::reference_wrapper<record_t>& record, std::vector<search_texts_t>& rules)
	{
		for (auto& rule : rules)
			rule.search(texts, record);
	}

	void find_and_replace_strategy_t::find_and_replace(rule_string& texts, std::reference_wrapper<record_t>& record, std::vector<search_texts_t>& rules)
	{
		for (auto& rule : rules)
		{
			rule.search(texts, record);
			if (!rule.empty())
			{
				replace_texts_t::replace(texts, record, rule.get_pattern(), rule.get_replacement());
			}
		}
	}
}
}