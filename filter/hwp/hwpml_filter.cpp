#include <filter_pch.h>
#include "hwp/hwpml_filter.h"
#include "editor/xml/xml_extract_texts.h"
#include "editor/xml/xml_editor.h"
#include <xlnt/detail/serialization/open_stream.hpp>
#include "locale/charset_encoder.h"

namespace filter
{
namespace hml
{
	typedef xml_traits::xml_document_t xml_document_t;
	typedef xml_traits::path_t path_t;
	using ::filter::xml::editor_t;

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
			editor_t editor;
			editor.extract(text_tag_name(), para_tag_name())
				.finalize(document);
			sections.emplace_back(editor.get_extract_result());
			return sections;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return sections_t();
	}

	void filter_t::replace_privacy(const rules_t& rules, char16_t replacement, std::unique_ptr<xml_document_t>& document)
	{
		try
		{
			editor_t editor;
			editor.extract(text_tag_name(), para_tag_name())
				.find(rules)
				.replace(replacement)
				.finalize(document);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	filter_t::sections_t filter_t::search_privacy(const rules_t& rules, std::unique_ptr<xml_document_t>& document)
	{
		try
		{
			editor_t editor;
			editor.extract(text_tag_name(), para_tag_name())
				.find(rules)
				.finalize(document);
			return editor.get_find_result();
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return sections_t();
	}
}
}