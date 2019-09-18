#pragma once
#include "traits/editor_traits.h"
#include "traits/xml_traits.h"

namespace filter
{
namespace xml
{
	class replace_texts_t
	{
	public:
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::rule_t rule_t;
		replace_texts_t();
		static void replace(para_t&& text, pugi::xml_node& node, const rule_t& pattern, char16_t replacement);
		static void replace(const std::string& text, pugi::xml_node& node, const rule_t& pattern, char16_t replacement);
		static void replace(pugi::xml_node& node, const rule_t& pattern, char16_t replace_dest);
	};
}
}