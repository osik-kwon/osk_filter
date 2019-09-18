#pragma once
#include "hwp/hwp50_syntax.h"
#include "traits/editor_traits.h"

namespace filter
{
namespace hwp50
{
	class replace_texts_t
	{
	public:
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rule_string rule_string;
		replace_texts_t();
		static void replace(rule_string& texts, std::reference_wrapper<record_t>& record, const rule_t& pattern, char16_t replacement);
	};
}
}