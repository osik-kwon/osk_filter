#include "filter_pch.h"
#include "word/doc_filter.h"
#include "locale/charset_encoder.h"
#include "io/compound_file_binary.h"

namespace filter
{
namespace doc
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
}
}