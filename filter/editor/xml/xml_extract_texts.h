#pragma once
#include <functional>
#include <map>
#include "traits/editor_traits.h"
#include "traits/xml_traits.h"
#include "editor/xml/xml_search_texts.h"

namespace filter
{
namespace xml
{
	class extract_texts_t : public pugi::xml_tree_walker
	{
	public:
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::rule_t rule_t;
		typedef int depth_t;
		typedef std::function<void(const std::string&, pugi::xml_node&, std::vector<search_texts_t>&)> ruler_t;
		std::map< depth_t, pugi::xml_node > para_nodes;

		extract_texts_t(const std::string& text_tag, const std::string& para_tag);
		extract_texts_t(const std::string& text_tag, const std::string& para_tag, std::map<uint32_t, std::string>* shared_strings);
		~extract_texts_t();
		virtual bool for_each(pugi::xml_node& node);
		extract_texts_t& make_rule(const rule_t& pattern, char16_t replacement = u'*');
		void change_rule(ruler_t that);
		const std::vector<search_texts_t>& get_rules() const {
			return rules;
		}
		section_t section;
	private:
		void end_paragraph_element();
		bool lookup_break() const;

		std::vector<search_texts_t> rules;
		ruler_t ruler;
		std::string text_tag;
		std::string para_tag;

		std::map<uint32_t, std::string>* shared_strings;
		bool has_shared;
	};
}
}