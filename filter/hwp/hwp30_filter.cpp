#include "filter_pch.h"
#include <fstream>
#include "hwp/hwp30_filter.h"
#include "hwp/hwp30_syntax.h"

#include "io/binary_iostream.h"
#include "io/zlib.h"

namespace filter
{
namespace hwp30
{
	filter_t::buffer_t filter_t::read_file(const std::string& path)
	{
		std::ifstream file(path, std::ios::binary);
		file.unsetf(std::ios::skipws);
		file.seekg(0, std::ios::end);
		std::streampos size = file.tellg();
		file.seekg(0, std::ios::beg);

		buffer_t buffer;
		buffer.reserve(std::size_t(size));
		std::copy(std::istream_iterator<byte_t>(file), std::istream_iterator<byte_t>(), std::back_inserter(buffer));
		return buffer;
	}

	void filter_t::parse_header(bufferstream& stream, std::unique_ptr<document_t>& document)
	{
		document->signature = binary_io::read(stream, 30);
		stream >> document->doc_info;
		stream >> document->doc_summary;	
		if (document->doc_info.info_block_length != 0)
		{
			document->info_block.info_block_length = document->doc_info.info_block_length;
			stream >> document->info_block;
		}
	}
	
	buffer_t filter_t::extract_body(buffer_t& buffer, bufferstream& stream, std::unique_ptr<document_t>& document)
	{
		streamsize stream_cur = stream.tellg();
		streamsize stream_end = buffer.size();
		streamsize body_size = stream_end - stream_cur;
		buffer_t body = binary_io::read(stream, body_size);
		if (document->doc_info.compressed != 0) // decompress : 0
			return hwp_zip::decompress_noexcept(body);
		return body;
	}

	void filter_t::parse_body(bufferstream& stream, std::unique_ptr<document_t>& document)
	{
		stream >> document->face_name_list;
		stream >> document->style_list;
	}

	std::unique_ptr<document_t> filter_t::parse(buffer_t& buffer)
	{
		std::unique_ptr<document_t> document(std::make_unique<document_t>());
		bufferstream import_stream(&buffer[0], buffer.size());
		parse_header(import_stream, document);
		buffer_t body = extract_body(buffer, import_stream, document);
		bufferstream body_stream(&body[0], body.size());
		parse_body(body_stream, document);
		return document;
	}

	filter_t::sections_t filter_t::extract_all_texts(const std::string& import_path)
	{
		try
		{
			sections_t sections;
			auto buffer = read_file(import_path);
			auto document = parse(buffer);
			// TODO: implement
			return sections;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return sections_t();
	}
}
}