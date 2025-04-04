#include "filter_pch.h"
#include "editor/txt/txt_replace_texts.h"

namespace filter
{
namespace txt
{
	replace_texts_t::replace_texts_t()
	{}

	void replace_texts_t::replace(rule_string& texts, para_ref_t& para_ref, const rule_t& pattern, char16_t replacement)
	{
		std::match_results<para_t::iterator> results;
		auto begin = texts.begin();
		while (std::regex_search(begin, texts.end(), results, pattern))
		{
			for (auto i = results[0].first; i != results[0].second; ++i)
			{
				*i = replacement;
			}
			begin += results.position() + results.length();
		}
	}
}
}