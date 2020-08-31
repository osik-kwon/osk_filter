#include "filter_pch.h"
#include "editor/hwp50/hwp50_editor.h"
#include "editor/hwp50/hwp50_extract_texts.h"
#include "editor/hwp50/hwp50_find_and_replace_strategy.h"
#include "locale/charset_encoder.h"

namespace filter
{
namespace hwp50
{
	editor_t::editor_t() : strategy(nullptr), replacement(0)
	{}

	editor_t& editor_t::extract(records_t&& that)
	{
		records = std::move(that);
		for (auto& record : records)
		{
			if (syntax_t::is_para_text(record.header.tag))
				para_text_record_refs.push_back(record);
		}
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

	void editor_t::write_records(std::unique_ptr<consumer_t>& consumer, std::unique_ptr<buffer_t>& src)
	{
		auto write_size = std::accumulate(records.begin(), records.end(), 0, [](size_t size, auto& record) {
			return size + record.size(); });
		buffer_t dest;
		dest.resize(write_size);
		bufferstream write_records_stream(&dest[0], dest.size());
		consumer->write_records(write_records_stream, records);
		std::swap(*src.get(), dest);
	}

	editor_t& editor_t::finalize(std::unique_ptr<consumer_t>& consumer, std::unique_ptr<buffer_t>& src)
	{
		try
		{
			if (src->empty())
				return *this;
			strategy = std::make_unique<extract_texts_t>();
			for (auto& rule : rules)
			{
				strategy->make_rule(rule);
			}
			if (!rules.empty())
				strategy->change_rule(find_and_replace_strategy_t::find_only);
			if (replacement != 0)
				strategy->change_rule(find_and_replace_strategy_t::find_and_replace);

			strategy->run(para_text_record_refs);
			if (!rules.empty())
				write_records(consumer, src);
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