#pragma once
#include <functional>
#include <map>
#include "traits/editor_traits.h"
#include "editor/hwp50/hwp50_search_texts.h"

namespace filter
{
namespace hwp50
{
	class editor_t;
	class extract_texts_t
	{
	public:
		friend class editor_t;
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rule_string rule_string;
		typedef std::vector<std::reference_wrapper<record_t>> record_refs_t;
		typedef std::function<void(rule_string&, std::reference_wrapper<record_t>&, std::vector<search_texts_t>&)> ruler_t;
		extract_texts_t();
		~extract_texts_t();
		extract_texts_t& make_rule(const rule_t& pattern, char16_t replacement = u'*');
		void change_rule(ruler_t that);
		const std::vector<search_texts_t>& get_rules() const {
			return rules;
		}
		void run(record_refs_t& record_refs);
	private:
		section_t section;
		std::vector<search_texts_t> rules;
		ruler_t ruler;
	};
}
}