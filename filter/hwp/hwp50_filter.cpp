#include "filter_pch.h"
#include "hwp/hwp50_filter.h"
#include "locale/charset_encoder.h"
#include "io/compound_file_binary.h"
#include "io/zlib.h"
#include "editor/hwp50/hwp50_editor.h"
#include "editor/hwp50/hwp50_extract_texts.h"

namespace filter
{
namespace hwp50
{
	std::unique_ptr<consumer_t> filter_t::open(const std::string& path)
	{
		try
		{
			std::unique_ptr<consumer_t> consumer = std::make_unique<consumer_t>();
			consumer->open(path);
			return consumer;

		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return std::make_unique<consumer_t>();
	}

	void filter_t::save(const std::string& path, std::unique_ptr<consumer_t>& consumer)
	{
		try
		{
			producer_t producer;
			producer.save(path, consumer);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	filter_t::sections_t filter_t::extract_all_texts(std::unique_ptr<consumer_t>& consumer)
	{
		try
		{
			sections_t sections;
			auto& entries = consumer->get_streams();
			for (auto entry = entries.begin(); entry != entries.end(); ++entry)
			{
				if (!consumer->has_paragraph(entry->first))
					continue;
				auto& section = entry->second;
				bufferstream stream(&section->at(0), section->size());
				editor_t editor;
				editor.extract(consumer->read_records(stream))
					.finalize(consumer, section);
				sections.emplace_back(editor.get_extract_result());
			}
			return sections;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return sections_t();
	}

	void filter_t::replace_privacy(const rules_t& rules, char16_t replacement, std::unique_ptr<consumer_t>& consumer)
	{
		try
		{
			auto& entries = consumer->get_streams();
			for (auto entry = entries.begin(); entry != entries.end(); ++entry)
			{
				if (!consumer->has_paragraph(entry->first))
					continue;
				auto& section = entry->second;
				bufferstream stream(&section->at(0), section->size());
				editor_t editor;
				editor.extract( consumer->read_records(stream) )
					.find(rules)
					.replace(replacement)
					.finalize(consumer, section);
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}
}
}