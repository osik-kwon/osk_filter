#include "filter_pch.h"
#include "txt/txt_filter.h"
#include "editor/txt/txt_editor.h"
#include "editor/txt/txt_extract_texts.h"

namespace filter
{
namespace txt
{
	filter_t::filter_t()
	{}

	std::string filter_t::detect_charset(const std::string& path)
	{
		return consumer_t::detect_charset(path);
	}

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

	void filter_t::save(const std::string& path, std::unique_ptr<consumer_t>& consumer, std::string charset)
	{
		try
		{
			custom_params_t params;
			if(!charset.empty())
				params.charset = charset;
			producer_t producer;
			producer.save(path, consumer, params);
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
			editor_t editor;
			editor.extract(consumer)
				.finalize();
			sections.emplace_back(editor.get_extract_result());
			return sections;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return sections_t();
	}

	filter_t::sections_t filter_t::search_privacy(const rules_t& rules, std::unique_ptr<consumer_t>& consumer)
	{
		try
		{
			sections_t sections;
			sections.resize(1);
			editor_t editor;
			editor.extract(consumer)
				.find(rules)
				.finalize();
			auto find_result = editor.get_find_result();
			std::copy(find_result.begin(), find_result.end(), std::back_inserter(sections));
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
			editor_t editor;
			editor.extract(consumer)
				.find(rules)
				.replace(replacement)
				.finalize();
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}
}
}