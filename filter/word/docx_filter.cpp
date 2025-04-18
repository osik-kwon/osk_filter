#include <filter_pch.h>
#include <word/docx_filter.h>
#include <functional>
#include <map>
#include "locale/charset_encoder.h"
#include "editor/xml/xml_extract_texts.h"
#include "editor/xml/xml_editor.h"

namespace filter
{
namespace docx
{
	typedef xml_traits::xml_document_t xml_document_t;
	typedef xml_traits::path_t path_t;
	typedef xml_traits::izstream_t izstream_t;
	typedef xml_traits::ozstream_t ozstream_t;
	using ::filter::xml::editor_t;

	filter_t::filter_t()
	{}

	std::regex filter_t::section_name_regex() const {
		return std::regex("word/document\\d*.xml");
	}

	std::string filter_t::text_tag_name() const {
		return std::string("w:t");
	}

	std::string filter_t::para_tag_name() const {
		return std::string("w:p");
	}

	std::unique_ptr<consumer_t> filter_t::open(const std::string& path)
	{
		try
		{
			std::unique_ptr<consumer_t> consumer = std::make_unique<consumer_t>();
			consumer->open(path_t(path));
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
			producer.save(path_t(path), consumer);
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
			const std::regex regex = section_name_regex();
			for (auto& part : consumer->get_parts())
			{
				auto document = part.second.get();
				if (!document)
					continue;
				if (std::regex_match(part.first, regex))
				{
					editor_t editor;
					editor.extract(text_tag_name(), para_tag_name())
						.finalize(document);
					sections.emplace_back(editor.get_extract_result());
				}
			}
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
			const std::regex regex = section_name_regex();
			for (auto& name : consumer->get_names())
			{
				auto document = consumer->get_part(name);
				if (!document)
					continue;
				if (std::regex_match(name.string(), regex))
				{
					editor_t editor;
					editor.extract(text_tag_name(), para_tag_name())
						.find(rules)
						.finalize(document);
					auto find_result = editor.get_find_result();
					std::copy(find_result.begin(), find_result.end(), std::back_inserter(sections));
				}
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
			const std::regex regex = section_name_regex();
			for (auto& name : consumer->get_names())
			{
				auto document = consumer->get_part(name);
				if (!document)
					continue;
				if (std::regex_match(name.string(), regex))
				{
					editor_t editor;
					editor.extract(text_tag_name(), para_tag_name())
						.find(rules)
						.replace(replacement)
						.finalize(document);
				}
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}
}
}