#include "filter_pch.h"
#include "editor/hwp30/hwp30_editor.h"
#include "editor/hwp30/hwp30_extract_texts.h"
#include "editor/hwp30/hwp30_find_and_replace_strategy.h"
#include "locale/charset_encoder.h"

namespace filter
{
namespace hwp30
{
	editor_t::editor_t() : strategy(nullptr), replacement(0)
	{}

	editor_t& editor_t::extract(std::unique_ptr<consumer_t>& consumer)
	{
		auto& document = consumer->get_document();
		if (!document)
			throw std::runtime_error("document is not exist");
		extractor_t::extract_para_list_ref(document->body.sections, para_list_ref);
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

	editor_t& editor_t::finalize()
	{
		try
		{
			strategy = std::make_unique<extract_texts_t>();
			for (auto& rule : rules)
			{
				strategy->make_rule(rule);
			}
			if (!rules.empty())
				strategy->change_rule(find_and_replace_strategy_t::find_only);
			if (replacement != 0)
				strategy->change_rule(find_and_replace_strategy_t::find_and_replace);

			strategy->run(para_list_ref);
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