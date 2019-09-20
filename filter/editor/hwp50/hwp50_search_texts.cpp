#include "filter_pch.h"
#include "editor/hwp50/hwp50_search_texts.h"
#include "locale/charset_encoder.h"

namespace filter
{
namespace hwp50
{
	search_texts_t::search_texts_t(const rule_t& pattern, char16_t replacement)
		: pattern(pattern), replacement(replacement)
	{}

	void search_texts_t::search(rule_string& texts, std::reference_wrapper<record_t>& record)
	{
		std::match_results<para_t::iterator> match;

		section_t lists;
		auto begin = texts.begin();
		while (std::regex_search(begin, texts.end(), match, pattern))
		{
			begin += match.position() + match.length();
			auto text = match.str();
			text.push_back(L'\n'); // TODO: normalize
			lists.push_back(text);
		}
		results.emplace_back(std::make_pair(record, lists));
	}

	search_texts_t::section_t search_texts_t::results_to_section() const
	{
		section_t section;
		for (auto& result : results)
		{
			section.reserve(section.size() + result.second.size());
			std::copy(result.second.begin(), result.second.end(), std::back_inserter(section));
		}
		return section;
	}
}
}