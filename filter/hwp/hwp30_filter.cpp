#include "filter_pch.h"
#include <fstream>
#include "hwp/hwp30_filter.h"
#include "hwp/hwp30_syntax.h"
#include "locale/charset_encoder.h"

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
		document->header.signature = binary_io::read(stream, 30);
		stream >> document->header.doc_info;
		stream >> document->header.doc_summary;
		if (document->header.doc_info.info_block_length != 0)
		{
			document->header.info_block.info_block_length = document->header.doc_info.info_block_length;
			stream >> document->header.info_block;
		}
	}
	
	buffer_t filter_t::extract_body(buffer_t& buffer, bufferstream& stream, std::unique_ptr<document_t>& document)
	{
		streamsize stream_cur = stream.tellg();
		streamsize stream_end = buffer.size();
		streamsize body_size = stream_end - stream_cur;
		buffer_t body = binary_io::read(stream, body_size);
		if (document->header.doc_info.compressed != 0) // decompress : 0
			return hwp_zip::decompress_noexcept(body);
		return body;
	}

	void filter_t::parse_body(bufferstream& stream, std::unique_ptr<document_t>& document)
	{
		stream >> document->body.face_name_list;
		stream >> document->body.style_list;

		bool end_of_para = false;
		do{
			paragraph_t para;
			stream >> para;
			document->body.para_list.push_back(std::move(para));
			end_of_para = para.para_header.empty();
		} while (!end_of_para);
	}

	std::unique_ptr<document_t> filter_t::parse(buffer_t& buffer)
	{
		std::unique_ptr<document_t> document(std::make_unique<document_t>());
		bufferstream header_stream(&buffer[0], buffer.size());
		parse_header(header_stream, document);
		buffer_t body = extract_body(buffer, header_stream, document);
		bufferstream body_stream(&body[0], body.size());
		parse_body(body_stream, document);
		return document;
	}

	std::unique_ptr<document_t> filter_t::open(const std::string& open_path)
	{
		try
		{
			sections_t sections;
			auto import_buffer = read_file(open_path);
			return parse(import_buffer);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return std::unique_ptr<document_t>(std::make_unique<document_t>());
	}

	filter_t::sections_t filter_t::extract_all_texts(const std::string& import_path)
	{
		try
		{
			sections_t sections;
			sections.resize(1);
			auto document = open(import_path);
			for (auto& para_body : document->body.para_list)
			{
				para_t para;
				for (auto& hchar : para_body.hchars)
				{
					auto utf16 = to_utf16(hchar.utf32);
					if (utf16.size() == 1)
					{
						if (utf16[0] == 0x000d)
							para.push_back(L'\n');
						else
							para.push_back(utf16[0]);
					}
				}
				sections[0].push_back(std::move(para));
			}
			return sections;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return sections_t();
	}

	bool filter_t::save(const std::unique_ptr<document_t>& document, const std::string& save_path)
	{
		try
		{
			buffer_t header_buffer;
			header_buffer.resize(document->header.size());

			bufferstream header_stream(&header_buffer[0], header_buffer.size());
			header_stream << document->header;

			buffer_t body_buffer;
			body_buffer.resize(document->body.size());
			bufferstream body_stream(&body_buffer[0], body_buffer.size());
			body_stream << document->body;

			// compress
			if (document->header.doc_info.compressed != 0)
			{
				streamsize buffer_size = body_stream.tellp(); // IMPORTANT!
				body_buffer = hwp_zip::compress_noexcept((char*)& body_buffer[0], (std::size_t)buffer_size);
			}

			// tail
			buffer_t tail_buffer;
			tail_buffer.resize(document->tail.size());
			bufferstream tail_stream(&tail_buffer[0], tail_buffer.size());
			tail_stream << document->tail;

			std::ofstream fout(save_path, std::ios::out | std::ios::binary);
			fout.write((char*)& header_buffer[0], header_buffer.size());
			fout.write((char*)& body_buffer[0], body_buffer.size());
			fout.write((char*)& tail_buffer[0], tail_buffer.size());
			fout.close();

			return true;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return false;
	}
}
}