#include "filter_pch.h"
#include "editor/xml/editor.h"
#include "editor/xml/extract_texts.h"
#include "editor/xml/find_and_replace_strategy.h"
#include "locale/charset_encoder.h"

namespace filter
{
namespace xml
{
	editor_t::editor_t() : strategy(nullptr), replacement(0)
	{}

	editor_t& editor_t::extract(const std::string& text, const std::string& para)
	{
		text_tag = text;
		para_tag = para;
		return *this;
	}

	editor_t& editor_t::find(const rules_t& that)
	{
		rules = that;
		return *this;
	}

	editor_t& editor_t::replace(char16_t that)
	{
		replacement = that;
		return *this;
	}

	editor_t& editor_t::finalize(std::unique_ptr<xml_document_t>& document)
	{
		finalize(document.get());
		return *this;
	}

	editor_t& editor_t::finalize(xml_document_t* document)
	{
		try
		{
			if(!document)
				return *this;
			strategy = std::make_unique<extract_texts_t>(text_tag, para_tag);
			for (auto& rule : rules)
			{
				strategy->make_rule(rule);
			}
			if (!rules.empty())
				strategy->change_rule(find_and_replace_strategy_t::find_only);
			if (replacement != 0)
				strategy->change_rule(find_and_replace_strategy_t::find_and_replace);
			document->traverse(*strategy);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return *this;
	}

	editor_t::section_t editor_t::get_extract_result() {
		if (strategy)
			return strategy->section;
		return section_t();
	}

	editor_t::sections_t editor_t::get_find_result()
	{
		if (strategy && !strategy->get_rules().empty())
		{
			sections_t sections;
			auto& rules = strategy->get_rules();
			for (auto& rule : rules)
				sections.emplace_back(rule.results_to_section());
			return sections;
		}
		return sections_t();
	}
}
}