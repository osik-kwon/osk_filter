#pragma once
#include "editor/hwp50/hwp50_search_texts.h"

namespace filter
{
namespace hwp50
{
	struct find_and_replace_strategy_t
	{
		typedef editor_traits::rule_string rule_string;
		static void do_nothing(rule_string& texts, std::reference_wrapper<record_t>& record, std::vector<search_texts_t>& rules);
		static void find_only(rule_string& texts, std::reference_wrapper<record_t>& record, std::vector<search_texts_t>& rules);
		static void find_and_replace(rule_string& texts, std::reference_wrapper<record_t>& record, std::vector<search_texts_t>& rules);
	};
}
}