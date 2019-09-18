#pragma once
#include "traits/editor_traits.h"
#include "traits/xml_traits.h"

namespace filter
{
namespace xml
{
	class search_texts_t
	{
	public:
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::rule_t rule_t;
		typedef std::pair< pugi::xml_node, section_t > result_t;

		search_texts_t(const rule_t& pattern, char16_t replacement);
		section_t results_to_section() const;
		void search(const std::string& text, pugi::xml_node& node);
		bool empty() const {
			return results.empty();
		}
		const std::wregex& get_pattern() const {
			return pattern;
		}
		char16_t get_replacement() const {
			return replacement;
		}
		const std::vector< result_t >& get_results() const {
			return results;
		}
	private:
		std::vector< result_t > results;
		rule_t pattern;
		char16_t replacement;
	};
}
}