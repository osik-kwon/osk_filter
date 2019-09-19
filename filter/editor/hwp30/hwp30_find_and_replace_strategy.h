#pragma once
#include "editor/hwp30/hwp30_search_texts.h"

namespace filter
{
namespace hwp30
{
	struct find_and_replace_strategy_t
	{
		typedef editor_traits::rule_string rule_string;
		typedef std::vector< std::reference_wrapper<hchar_t> > para_ref_t;
		typedef std::vector<para_ref_t> para_list_ref_t;
		static void do_nothing(rule_string& texts, para_list_ref_t& para_list_ref, std::vector<search_texts_t>& rules);
		static void find_only(rule_string& texts, para_list_ref_t& para_list_ref, std::vector<search_texts_t>& rules);
		static void find_and_replace(rule_string& texts, para_list_ref_t& para_list_ref, std::vector<search_texts_t>& rules);
	};
}
}