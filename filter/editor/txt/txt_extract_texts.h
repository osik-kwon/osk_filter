#pragma once
#include <functional>
#include <map>
#include "traits/editor_traits.h"
#include "editor/txt/txt_search_texts.h"

namespace filter
{
namespace txt
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
		typedef std::reference_wrapper<std::wstring> para_ref_t;
		typedef std::vector<para_ref_t> para_list_ref_t;
		typedef std::function<void(rule_string&, para_ref_t&, std::vector<search_texts_t>&)> ruler_t;
		extract_texts_t();
		~extract_texts_t();
		extract_texts_t& make_rule(const rule_t& pattern, char16_t replacement = u'*');
		void change_rule(ruler_t that);
		const std::vector<search_texts_t>& get_rules() const {
			return rules;
		}
		void run(para_list_ref_t& para_list_ref);
	private:
		section_t section;
		std::vector<search_texts_t> rules;
		ruler_t ruler;
	};
}
}