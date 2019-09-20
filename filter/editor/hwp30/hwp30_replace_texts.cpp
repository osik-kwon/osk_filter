#include "filter_pch.h"
#include "editor/hwp30/hwp30_replace_texts.h"

namespace filter
{
namespace hwp30
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
				auto pos = std::distance(texts.begin(), i);
				if (para_ref.size() > pos && pos >= 0)
					para_ref[pos].get().code = replacement;
				// TODO: log
			}
			begin += results.position() + results.length();
		}
	}
}
}