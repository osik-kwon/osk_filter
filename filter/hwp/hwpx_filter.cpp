#include <filter_pch.h>
#include <hwp/hwpx_filter.h>
#include <xlnt/detail/serialization/open_stream.hpp>

namespace filter
{
namespace hwpx
{
	typedef xml_traits::xml_document_t xml_document_t;
	typedef xml_traits::path_t path_t;
	typedef xml_traits::izstream_t izstream_t;
	typedef xml_traits::ozstream_t ozstream_t;

	filter_t::filter_t()
	{}

	filter_t::sections_t filter_t::extract_all_texts(const std::string& path)
	{
		try
		{
			sections_t sections;
			consumer_t consumer;
			consumer.open(path_t(path));



			return sections;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return sections_t();
	}

	consumer_t::consumer_t()
	{}

	std::unique_ptr<izstream_t> consumer_t::open_package(const path_t& path)
	{
		std::ifstream source;
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
		auto document = extract_part(path, izstream);
		parts.emplace(std::move(path.string()), std::move(document));
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
}
}