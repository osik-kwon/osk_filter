#pragma once
#include "traits/editor_traits.h"

namespace filter
{
namespace txt
{
	class replace_texts_t
	{
	public:
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rule_string rule_string;
		typedef std::reference_wrapper<std::wstring> para_ref_t;
		typedef std::vector<para_ref_t> para_list_ref_t;
		replace_texts_t();
		static void replace(rule_string& texts, para_ref_t& para_ref, const rule_t& pattern, char16_t replacement);
	};
}
}