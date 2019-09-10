#include <filter_pch.h>
#include "hwp/hwpx_filter.h"
#include <functional>
#include <map>
#include "locale/charset_encoder.h"
#include "io/open_package_conventions.h"

namespace filter
{
namespace hwpx
{
	typedef xml_traits::xml_document_t xml_document_t;
	typedef xml_traits::path_t path_t;
	typedef xml_traits::izstream_t izstream_t;
	typedef xml_traits::ozstream_t ozstream_t;

	struct extract_texts_t : pugi::xml_tree_walker
	{
		extract_texts_t();
		typedef filter_t::section_t section_t;
		typedef int depth_t;
		std::map< depth_t, pugi::xml_node > para_nodes;

		virtual bool for_each(pugi::xml_node& node);
		section_t section;
	protected:
		virtual void replace_privacy(const std::string& text, pugi::xml_node& node){}
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
		if (name == "hp:t")
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
		else if (name == "hp:p")
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

	std::regex filter_t::section_name_regex() const {
		return std::regex("Contents/section\\d*.xml");
	}

	void filter_t::replace_privacy(const std::wregex& pattern, char16_t replace_dest, std::unique_ptr<consumer_t>& consumer)
	{
		try
		{
			const std::regex regex = section_name_regex();
			replace_texts_t replace_texts(pattern, replace_dest);
			for (auto& name : consumer->get_names())
			{
				auto document = consumer->get_part(name);
				if (!document)
					continue;
				if (std::regex_match(name.string(), regex))
				{
					document->traverse(replace_texts);
				}
			}
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
			extract_texts_t extract_texts;
			for (auto& name : consumer->get_names())
			{
				auto document = consumer->get_part(name);
				if (!document)
					continue;
				if (std::regex_match(name.string(), regex))
				{
					document->traverse(extract_texts);
					sections.emplace_back(std::move(extract_texts.section));
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
}
}