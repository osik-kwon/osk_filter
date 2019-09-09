#include <filter_pch.h>
#include "hwp/hwpx_filter.h"
#include <functional>
#include <map>
#include <xlnt/detail/serialization/open_stream.hpp>
#include "locale/charset_encoder.h"

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
		extract_texts_t()
		{
			section.resize(1); // IMPORTANT!
		}
		typedef filter_t::section_t section_t;
		typedef int depth_t;
		std::map< depth_t, std::reference_wrapper<pugi::xml_node> > para_nodes;

		virtual bool for_each(pugi::xml_node& node)
		{
			end_element();
			auto name = std::string(node.name());
			if (name == "hp:t")
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
				}
			}
			else if (name == "hp:p")
			{
				if( para_nodes.find(depth()) != para_nodes.end() )
					throw std::runtime_error("invalid para end");
				para_nodes.insert( std::make_pair(depth(), std::ref(node) ));
			}
			return true;
		}
		section_t section;
	private:
		void end_element()
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

		bool lookup_break() const
		{
			if (!section.empty() && !section.back().empty())
			{
				return section.back().back() == L'\n';
			}
			return false;
		}
	};

	filter_t::filter_t()
	{}

	std::regex filter_t::section_name_regex() const {
		return std::regex("Contents/section\\d*.xml");
	}

	filter_t::sections_t filter_t::extract_all_texts(const std::string& path)
	{
		try
		{
			sections_t sections;
			sections.resize(1);
			consumer_t consumer;
			consumer.open(path_t(path));

			const std::regex regex = section_name_regex();
			extract_texts_t extract_texts;
			for (auto& name : consumer.get_names())
			{
				auto document = consumer.get_part(name);
				if (!document)
					continue;			
				if( std::regex_match(name.string(), regex) )
					document->traverse(extract_texts);
			}
			sections[0] = std::move(extract_texts.section);
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

	consumer_t::consumer_t()
	{}

	std::unique_ptr<izstream_t> consumer_t::open_package(const path_t& path)
	{
		xlnt::detail::open_stream(source, path.string());
		if (!source.good())
			throw std::runtime_error("file not found : " + path.string());
		return std::make_unique<izstream_t>(source);
	}

	std::unique_ptr<xml_document_t> consumer_t::extract_part(const path_t& path, std::unique_ptr<izstream_t>& izstream)
	{
		auto stream_buf = izstream->open(path);
		std::istream stream(stream_buf.get());

		auto document = std::make_unique<xml_document_t>();
		pugi::xml_parse_result result = document->load(stream, pugi::parse_default, pugi::xml_encoding::encoding_auto);
		if (!result)
			throw std::runtime_error("xml load fail : " + path.string());
		return document;
	}

	void consumer_t::load_part(const path_t& path, std::unique_ptr<izstream_t>& izstream)
	{
		try
		{
			auto document = extract_part(path, izstream);
			parts.emplace(std::move(path.string()), std::move(document));
		}
		catch (const std::exception&)
		{}
	}

	void consumer_t::open(const path_t& path)
	{
		auto archive = open_package(path);
		names = archive->files();
		for (auto name : names)
		{
			load_part(name, archive);
		}
	}

	producer_t::producer_t()
	{}

	void producer_t::save(const path_t& path, std::unique_ptr<consumer_t>& consumer)
	{
		xlnt::detail::open_stream(dest, path.string());
		std::unique_ptr<ozstream_t> archive;
		archive.reset(new ozstream_t(dest));

		auto& files = consumer->get_names();
		for (auto file : files)
		{
			std::unique_ptr<std::streambuf> part_buf;
			part_buf.reset();
			part_buf = archive->open(file);

			std::ostream part_stream(nullptr);
			part_stream.rdbuf(part_buf.get());

			auto& parts = consumer->get_parts();
			auto part_doc = parts.find(file.string());
			if (part_doc == parts.end())
				continue;
			part_doc->second->save(part_stream, PUGIXML_TEXT("\t"), pugi::parse_default, pugi::xml_encoding::encoding_auto);
		}

		archive.reset(nullptr);
		dest.close();
	}
}
}