#pragma once
#include "editor/txt/txt_search_texts.h"

namespace filter
{
namespace txt
{
	struct find_and_replace_strategy_t
	{
		typedef editor_traits::rule_string rule_string;
		typedef std::reference_wrapper<std::wstring> para_ref_t;
		typedef std::vector<para_ref_t> para_list_ref_t;
		static void do_nothing(rule_string& texts, para_ref_t& para_ref, std::vector<search_texts_t>& rules);
		static void find_only(rule_string& texts, para_ref_t& para_ref, std::vector<search_texts_t>& rules);
		static void find_and_replace(rule_string& texts, para_ref_t& para_ref, std::vector<search_texts_t>& rules);
	};
}
}