#include <filter_pch.h>
#include "hwp/hwpml_filter.h"
#include <xlnt/detail/serialization/open_stream.hpp>
#include "locale/charset_encoder.h"

namespace filter
{
namespace hml
{
	typedef xml_traits::xml_document_t xml_document_t;
	typedef xml_traits::path_t path_t;

	class search_texts_t
	{
	public:
		typedef filter_t::para_t para_t;
		typedef filter_t::section_t section_t;
		typedef std::pair< pugi::xml_node, section_t > result_t;
		search_texts_t(const std::wregex& pattern, char16_t replacement);
		section_t results_to_section() const;
		void search(const std::string& text, pugi::xml_node& node);
		bool empty() const {
			return results.empty();
		}
		const std::wregex& get_pattern() const {
			return pattern;
		}
		char16_t get_replacement() const {
			return replacement;
		}
		const std::vector< result_t >& get_results() const {
			return results;
		}
	private:
		mutable std::vector< result_t > results;
		std::wregex pattern;
		char16_t replacement;
	};

	search_texts_t::search_texts_t(const std::wregex& pattern, char16_t replacement)
		: pattern(pattern), replacement(replacement)
	{}

	void search_texts_t::search(const std::string& text, pugi::xml_node& node)
	{
		para_t texts = to_wchar(text);
		std::match_results<para_t::iterator> match;

		section_t lists;
		auto begin = texts.begin();
		while (std::regex_search(begin, texts.end(), match, pattern))
		{
			begin += match.position() + match.length();
			lists.push_back(match.str());
		}
		results.emplace_back(std::make_pair(node, lists));
	}

	search_texts_t::section_t search_texts_t::results_to_section() const
	{
		section_t section;
		std::for_each(results.begin(), results.end(), [&section](result_t& result) {
			section.reserve(section.size() + result.second.size());
			std::copy(result.second.begin(), result.second.end(), std::back_inserter(section));
			});
		return section;
	}

	class replace_texts_t
	{
	public:
		typedef filter_t::para_t para_t;
		typedef filter_t::section_t section_t;
		replace_texts_t();
		static void replace(para_t&& text, pugi::xml_node& node, const std::wregex& pattern, char16_t replacement);
		static void replace(const std::string& text, pugi::xml_node& node, const std::wregex& pattern, char16_t replacement);
		static void replace(pugi::xml_node& node, const std::wregex& pattern, char16_t replace_dest);
	};

	replace_texts_t::replace_texts_t()
	{}

	void replace_texts_t::replace(para_t&& texts, pugi::xml_node& node, const std::wregex& pattern, char16_t replacement)
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

	void replace_texts_t::replace(const std::string& text, pugi::xml_node& node, const std::wregex& pattern, char16_t replacement)
	{
		replace(to_wchar(text), node, pattern, replacement);
	}

	void replace_texts_t::replace(pugi::xml_node& node, const std::wregex& pattern, char16_t replacement)
	{
		replace(node.first_child().value(), node, pattern, replacement);
	}

	struct find_and_replace_strategy_t
	{
		static void do_nothing(const std::string& text, pugi::xml_node& node, std::vector<search_texts_t>& rules);
		static void find_only(const std::string& text, pugi::xml_node& node, std::vector<search_texts_t>& rules);
		static void find_and_replace(const std::string& text, pugi::xml_node& node, std::vector<search_texts_t>& rules);
	};

	void find_and_replace_strategy_t::do_nothing(const std::string& text, pugi::xml_node& node, std::vector<search_texts_t>& rules)
	{}

	void find_and_replace_strategy_t::find_only(const std::string& text, pugi::xml_node& node, std::vector<search_texts_t>& rules)
	{
		for (auto& rule : rules)
			rule.search(text, node);
	}

	void find_and_replace_strategy_t::find_and_replace(const std::string& text, pugi::xml_node& node, std::vector<search_texts_t>& rules)
	{
		for (auto& rule : rules)
		{
			rule.search(text, node);
			if (!rule.empty())
			{
				replace_texts_t::replace(text, node, rule.get_pattern(), rule.get_replacement());
			}
		}
	}

	class extract_texts_t : public pugi::xml_tree_walker
	{
	public:
		typedef filter_t::section_t section_t;
		typedef int depth_t;
		typedef std::function<void(const std::string&, pugi::xml_node&, std::vector<search_texts_t>&)> ruler_t;
		std::map< depth_t, pugi::xml_node > para_nodes;

		extract_texts_t(const std::string& text_tag, const std::string& para_tag,
			ruler_t ruler = &find_and_replace_strategy_t::do_nothing);
		virtual bool for_each(pugi::xml_node& node);
		extract_texts_t& make_rule(const std::wregex& pattern, char16_t replacement = u'*');
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
	};

	extract_texts_t::extract_texts_t(const std::string& text_tag, const std::string& para_tag, ruler_t ruler)
		: text_tag(text_tag), para_tag(para_tag), ruler(ruler)
	{
		section.resize(1); // IMPORTANT!
	}

	extract_texts_t& extract_texts_t::make_rule(const std::wregex& pattern, char16_t replacement)
	{
		rules.emplace_back(search_texts_t(pattern, replacement));
		return *this;
	}

	void extract_texts_t::change_rule(ruler_t that) {
		ruler = that;
	}

	bool extract_texts_t::for_each(pugi::xml_node& node)
	{
		end_paragraph_element();
		auto name = std::string(node.name());
		if (name == text_tag)
		{
			std::string text(node.first_child().value());
			if (!text.empty())
			{
				if (lookup_break())
					section.emplace_back(std::move(to_wchar(text)));
				else
				{
					section.back() += to_wchar(text);
				}
				ruler(text, node, rules);
			}
		}
		else if (name == para_tag)
		{
			if (para_nodes.find(depth()) != para_nodes.end())
				throw std::runtime_error("invalid para end");
			para_nodes.insert(std::make_pair(depth(), node));
		}
		return true;
	}

	void extract_texts_t::end_paragraph_element()
	{
		if (!para_nodes.empty())
		{
			auto cur = depth();
			auto upper = para_nodes.upper_bound(cur);
			if (upper == para_nodes.end())
				upper = para_nodes.find(cur);
			while (upper != para_nodes.end())
			{
				if (upper != para_nodes.end())
				{
					upper = para_nodes.erase(upper);
					section.back().push_back(L'\n'); // TODO: normalize
				}
				else
					++upper;
			}
		}
	}

	bool extract_texts_t::lookup_break() const
	{
		if (!section.empty() && !section.back().empty())
		{
			return section.back().back() == L'\n';
		}
		return false;
	}

	filter_t::filter_t()
	{}

	std::unique_ptr<xml_document_t> filter_t::open(const std::string& path)
	{
		try
		{
			std::ifstream source;
			xlnt::detail::open_stream(source, path);
			if (!source.good())
				throw std::runtime_error("file not found : " + path);

			auto document = std::make_unique<xml_document_t>();
			pugi::xml_parse_result result = document->load(source, pugi::parse_default, pugi::xml_encoding::encoding_auto);
			if (!result)
				throw std::runtime_error("invalid xml format");
			return document;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return std::make_unique<xml_document_t>();
	}

	void filter_t::save(const std::string& path, std::unique_ptr<xml_document_t>& document)
	{
		try
		{
			std::ofstream dest;
			xlnt::detail::open_stream(dest, path);
			document->save(dest, PUGIXML_TEXT("\t"), pugi::parse_default, pugi::xml_encoding::encoding_auto);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	std::string filter_t::text_tag_name() const {
		return std::string("CHAR");
	}

	std::string filter_t::para_tag_name() const {
		return std::string("P");
	}

	filter_t::sections_t filter_t::extract_all_texts(std::unique_ptr<xml_document_t>& document)
	{
		try
		{
			sections_t sections;
			extract_texts_t strategy(text_tag_name(), para_tag_name(), &find_and_replace_strategy_t::do_nothing);
			document->traverse(strategy);
			sections.emplace_back(std::move(strategy.section));
			return sections;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return sections_t();
	}

	void filter_t::replace_privacy(const std::wregex& pattern, char16_t replacement, std::unique_ptr<xml_document_t>& document)
	{
		try
		{
			extract_texts_t strategy(text_tag_name(), para_tag_name(), &find_and_replace_strategy_t::find_and_replace);
			strategy.make_rule(pattern, replacement);
			document->traverse(strategy);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	filter_t::section_t filter_t::search_privacy(const std::wregex& pattern, std::unique_ptr<xml_document_t>& document)
	{
		try
		{
			section_t section;
			extract_texts_t strategy(text_tag_name(), para_tag_name(), &find_and_replace_strategy_t::find_only);
			strategy.make_rule(pattern);
			document->traverse(strategy);
			if(!strategy.get_rules().empty())
				section = strategy.get_rules().back().results_to_section();
			return section;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return section_t();
	}
}
}