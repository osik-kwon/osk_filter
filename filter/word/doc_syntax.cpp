#include "filter_pch.h"
#include "word/doc_syntax.h"
#include "io/compound_file_binary.h"

namespace filter
{
namespace doc
{
	consumer_t::consumer_t()
	{}

	void consumer_t::open(const std::string& path)
	{
		try
		{
			auto storage = cfb_t::make_read_only_storage(path);
			auto entries = cfb_t::make_full_entries(storage, "/");
			for (auto& entry : entries)
			{
				auto plain = cfb_t::extract_stream(storage, entry);

				// TODO: remove
				bufferstream stream(&plain[0], plain.size());
				FileInformationBlock record;
				stream >> record;

				// TODO:
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	producer_t::producer_t()
	{}

	void producer_t::save(const std::string& path, std::unique_ptr<consumer_t>& consumer)
	{
		try
		{
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}
}
}