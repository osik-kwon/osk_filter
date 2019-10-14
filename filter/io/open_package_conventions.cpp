#include "filter_pch.h"
#include "io/open_package_conventions.h"
#include "io/file_stream.h"
#include <xlnt/detail/serialization/vector_streambuf.hpp>

namespace filter
{
namespace opc
{
	typedef xml_traits::xml_document_t xml_document_t;
	typedef xml_traits::path_t path_t;
	typedef xml_traits::izstream_t izstream_t;
	typedef xml_traits::ozstream_t ozstream_t;

	consumer_t::consumer_t()
	{}

	consumer_t::~consumer_t()
	{}

	std::unique_ptr<izstream_t> consumer_t::open_package(const path_t& path)
	{
		source.open(to_fstream_path(path.string()), std::ios::binary);
		source.exceptions(std::ifstream::badbit | std::ifstream::failbit);
		return std::make_unique<izstream_t>(source);
	}

	void consumer_t::load_part(const path_t& path, std::unique_ptr<izstream_t>& izstream)
	{
		auto stream_buf = izstream->open(path);
		std::istream stream(stream_buf.get());

		auto document = std::make_unique<xml_document_t>();
		pugi::xml_parse_result result = document->load(stream, pugi::parse_default, pugi::xml_encoding::encoding_auto);
		if (result)
		{
			if (parts.find(path.string()) != parts.end())
				throw std::runtime_error("invalid parts");
			parts.emplace(std::move(path.string()), std::move(document));
		}
		else
		{
			if (buffers.find(path.string()) != buffers.end())
				throw std::runtime_error("invalid buffers buffer");

			buffer_t buffer;
			auto streambuf = izstream->open(path);
			xlnt::detail::vector_ostreambuf ostreambuf(buffer);
			std::ostream ostream(&ostreambuf);
			ostream << streambuf.get();
			buffers.emplace(std::move(path.string()), std::move(buffer));
		}
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

	producer_t::~producer_t()
	{}

	void producer_t::save(const path_t& path, std::unique_ptr<consumer_t>& consumer)
	{
		dest.open(to_fstream_path(path.string()), std::ios::binary);
		dest.exceptions(std::ofstream::badbit | std::ofstream::failbit);

		std::unique_ptr<ozstream_t> archive(new ozstream_t(dest));
		auto& files = consumer->get_names();
		for (auto file : files)
		{
			std::unique_ptr<std::streambuf> part_buf;
			part_buf.reset();
			part_buf = archive->open(file);

			auto& parts = consumer->get_parts();
			auto part_doc = parts.find(file.string());
			if (part_doc != parts.end())
			{
				std::ostream part_stream(part_buf.get());
				part_doc->second->save(part_stream, PUGIXML_TEXT("\t"), pugi::parse_default, pugi::xml_encoding::encoding_auto);
			}

			auto& buffers = consumer->get_buffers();
			auto part_buffer = buffers.find(file.string());
			if (part_buffer != buffers.end())
			{
				xlnt::detail::vector_istreambuf buffer(part_buffer->second);
				std::ostream(part_buf.get()) << &buffer;
			}
		}
	}
}
}