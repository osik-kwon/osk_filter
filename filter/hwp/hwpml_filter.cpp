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

	struct extract_texts_t : pugi::xml_tree_walker
	{
		extract_texts_t();
		typedef filter_t::section_t section_t;
		typedef int depth_t;
		std::map< depth_t, std::reference_wrapper<pugi::xml_node> > para_nodes;

		virtual bool for_each(pugi::xml_node& node);
		section_t section;
	protected:
		virtual void replace_privacy(const std::string& text, pugi::xml_node& node) {}
	private:
		void end_paragraph_element();
		bool lookup_break() const;
	};

	extract_texts_t::extract_texts_t()
	{
		section.resize(1); // IMPORTANT!
	}

	bool extract_texts_t::for_each(pugi::xml_node& node)
	{
		end_paragraph_element();
		auto name = std::string(node.name());
		if (name == "CHAR")
		{
			std::string text(node.first_child().value());
			if (!text.empty())
			{
				replace_privacy(text, node);
				if (lookup_break())
					section.emplace_back(std::move(to_wchar(text)));
				else
				{
					section.back() += to_wchar(text);
				}
			}
		}
		else if (name == "P")
		{
			if (para_nodes.find(depth()) != para_nodes.end())
				throw std::runtime_error("invalid para end");
			para_nodes.insert(std::make_pair(depth(), std::ref(node)));
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

	struct replace_texts_t : extract_texts_t
	{
		replace_texts_t(const std::wregex& pattern, char16_t replace_dest);
	protected:
		virtual void replace_privacy(const std::string& text, pugi::xml_node& node);
	private:
		std::wregex pattern;
		char16_t replace_dest;
	};

	replace_texts_t::replace_texts_t(const std::wregex& pattern, char16_t replace_dest)
		: pattern(pattern), replace_dest(replace_dest)
	{}

	void replace_texts_t::replace_privacy(const std::string& text, pugi::xml_node& node)
	{
		std::wstring texts = to_wchar(text);
		std::match_results<std::wstring::iterator> results;
		auto begin = texts.begin();
		while (std::regex_search(begin, texts.end(), results, pattern))
		{
			for (auto i = results[0].first; i != results[0].second; ++i)
			{
				*i = replace_dest;
			}
			begin += results.position() + results.length();
		}
		node.first_child().set_value(to_utf8(texts).c_str());
	}

	filter_t::filter_t()
	{}

	filter_t::~filter_t()
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

	filter_t::sections_t filter_t::extract_all_texts(std::unique_ptr<xml_document_t>& document)
	{
		try
		{
			sections_t sections;
			sections.resize(1);

			extract_texts_t extract_texts;
			document->traverse(extract_texts);
			sections.emplace_back(std::move(extract_texts.section));
			return sections;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return sections_t();
	}

	void filter_t::replace_privacy(const std::wregex& pattern, char16_t replace_dest, std::unique_ptr<xml_document_t>& document)
	{
		try
		{
			replace_texts_t replace_texts(pattern, replace_dest);
			document->traverse(replace_texts);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}
}
}