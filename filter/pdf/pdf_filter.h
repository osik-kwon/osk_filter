#pragma once
#include <string>
#include "traits/editor_traits.h"

namespace filter
{
namespace pdf
{
	class filter_t
	{
	public:
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::sections_t sections_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rules_t rules_t;

		filter_t() = default;
		std::string open(const std::string& path); // dummy
		sections_t extract_all_texts(const std::string& path);
	private:
	};
}
}