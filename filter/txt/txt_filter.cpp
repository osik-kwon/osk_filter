#include "filter_pch.h"
#include "txt/txt_filter.h"
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/newline.hpp>
#include <boost/iostreams/device/file.hpp>

namespace filter
{
namespace txt
{
	consumer_t::consumer_t()
	{}

	void consumer_t::open(const std::string& path)
	{
		try
		{
			boost::iostreams::filtering_istream in;
			in.push(boost::iostreams::newline_filter(boost::iostreams::newline::posix));
			in.push(boost::iostreams::file_source(path));

			document.push_back(para_t());
			while (std::getline(in, document.back()));
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}	
	}

	producer_t::producer_t()
	{}

	void producer_t::save(const std::string& path, std::unique_ptr<consumer_t>& consumer)
	{}

	filter_t::filter_t()
	{}

	std::unique_ptr<consumer_t> filter_t::open(const std::string& path)
	{
		return std::make_unique<consumer_t>();
	}
	void filter_t::save(const std::string& path, std::unique_ptr<consumer_t>& consumer)
	{
	}

	filter_t::sections_t filter_t::extract_all_texts(std::unique_ptr<consumer_t>& consumer)
	{
		sections_t sections;
		return sections;
	}

	filter_t::sections_t filter_t::search_privacy(const rules_t& rules, std::unique_ptr<consumer_t>& consumer)
	{
		sections_t sections;
		return sections;
	}

	void filter_t::replace_privacy(const rules_t& rules, char16_t replacement, std::unique_ptr<consumer_t>& consumer)
	{}
}
}