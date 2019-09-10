#pragma once
#include <string>
#include <vector>
#include <regex>

namespace filter
{
	struct editor_traits
	{
		typedef std::wstring para_t;
		typedef std::vector<para_t> section_t;
		typedef std::vector<section_t> sections_t;
		typedef std::wregex rule_t;
		typedef std::vector<std::wregex> rules_t;
	};
}