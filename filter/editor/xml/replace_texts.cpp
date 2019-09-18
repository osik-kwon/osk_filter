#include "filter_pch.h"
#include "editor/xml/replace_texts.h"
#include "locale/charset_encoder.h"

namespace filter
{
namespace xml
{
	replace_texts_t::replace_texts_t()
	{}

	void replace_texts_t::replace(para_t&& texts, pugi::xml_node& node, const rule_t& pattern, char16_t replacement)
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
		node.first_child().set_value(to_utf8(texts).c_str());
	}

	void replace_texts_t::replace(const std::string& text, pugi::xml_node& node, const rule_t& pattern, char16_t replacement)
	{
		replace(to_wchar(text), node, pattern, replacement);
	}

	void replace_texts_t::replace(pugi::xml_node& node, const rule_t& pattern, char16_t replacement)
	{
		replace(node.first_child().value(), node, pattern, replacement);
	}
}
}