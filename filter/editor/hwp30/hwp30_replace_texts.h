#pragma once
#include "hwp/hwp30_syntax.h"
#include "traits/editor_traits.h"

namespace filter
{
namespace hwp30
{
	class replace_texts_t
	{
	public:
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rule_string rule_string;
		typedef std::vector< std::reference_wrapper<hchar_t> > para_ref_t;
		typedef std::vector<para_ref_t> para_list_ref_t;
		replace_texts_t();
		static void replace(rule_string& texts, para_list_ref_t& para_list_ref, const rule_t& pattern, char16_t replacement);
	};
}
}