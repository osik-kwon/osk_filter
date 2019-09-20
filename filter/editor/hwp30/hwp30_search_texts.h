#pragma once
#include "hwp/hwp30_syntax.h"
#include "traits/editor_traits.h"

namespace filter
{
namespace hwp30
{
	class search_texts_t
	{
	public:
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rule_string rule_string;
		typedef std::vector< std::reference_wrapper<hchar_t> > para_ref_t;
		typedef std::vector<para_ref_t> para_list_ref_t;
		typedef std::pair< para_ref_t, section_t > result_t;

		search_texts_t(const rule_t& pattern, char16_t replacement);
		section_t results_to_section() const;
		void search(rule_string& texts, para_ref_t& para_ref);
		bool empty() const {
			return results.empty();
		}
		const rule_t& get_pattern() const {
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