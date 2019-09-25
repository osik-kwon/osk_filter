#include "filter_pch.h"
#include "editor/txt/txt_extract_texts.h"
#include "editor/txt/txt_find_and_replace_strategy.h"
#include "locale/charset_encoder.h"

namespace filter
{
namespace txt
{	
	extract_texts_t::extract_texts_t() : ruler(&find_and_replace_strategy_t::do_nothing)
	{}

	extract_texts_t::~extract_texts_t()
	{}

	extract_texts_t& extract_texts_t::make_rule(const rule_t& pattern, char16_t replacement)
	{
		rules.emplace_back(search_texts_t(pattern, replacement));
		return *this;
	}

	void extract_texts_t::change_rule(ruler_t that) {
		ruler = that;
	}

	void extract_texts_t::run(para_list_ref_t& para_list_ref)
	{
		for (auto& para : para_list_ref)
			ruler(para.get(), para, rules);

		for (auto& para : para_list_ref)
		{
			section.push_back(para.get());
			section.back().push_back(L'\n'); // TODO: normalize
		}
	}
}
}